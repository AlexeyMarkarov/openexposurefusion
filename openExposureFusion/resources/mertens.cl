constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

/*emulate clamp() and mix() for integers*/
int2 borderCoord(int2 coord, const int2 maxCoord)
{
    coord = convert_int2(abs(coord));
    const int2 minValue = maxCoord * (int2)(2, 2) - coord;
    return minValue + (coord - minValue) * max(min(maxCoord - coord, (int2)(1, 1)), (int2)(0, 0));
}

#define GRAY (float4)(0.299f, 0.587f, 0.114f, 0.0f)

kernel void krn_weight(const int2 kernelSize, read_only image2d_t image, write_only image2d_t weightMap,
    const float3 params, const int2 maxCoord)
/* params: x = contrast, y = saturation, z = exposedness */
/* laplace filter:
    0.0f,    1.0f,    0.0f,
    1.0f,    -4.0f,   1.0f,
    0.0f,    1.0f,    0.0f
*/
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    const float4 srcColor = read_imagef(image, sampler, coord);
    float3 measures = (float3)(1.0f);

    /*calculate contrast measure - apply laplacian filter on grayscale image*/
    measures.x = fabs(dot(srcColor, GRAY) * -4.0f
        + dot(read_imagef(image, sampler, borderCoord(coord + (int2)(0, -1), maxCoord)), GRAY)
        + dot(read_imagef(image, sampler, borderCoord(coord + (int2)(-1, 0), maxCoord)), GRAY)
        + dot(read_imagef(image, sampler, borderCoord(coord + (int2)(1, 0), maxCoord)), GRAY)
        + dot(read_imagef(image, sampler, borderCoord(coord + (int2)(0, 1), maxCoord)), GRAY));

    /*calculate saturation measure - distance between original color and mean color value*/
    measures.y = fast_length(srcColor.xyz - (float3)(dot(srcColor.xyz, (float3)(0.3333333333f))));

    /*calculate exposedness measure*/
    float8 tmp;
    tmp.s012 = srcColor.xyz - (float3)(0.5f);
    tmp.s456 = -(tmp.s012 * tmp.s012) * (float3)(12.5f);
    measures.z = tmp.s4 * tmp.s5 * tmp.s6;

    /*apply coefficients*/
    measures = pow(measures, params);

    write_imagef(weightMap, coord, (float4)(measures.x * measures.y * measures.z));
}

kernel void krn_add(const int2 kernelSize, read_only image2d_t src1, read_only image2d_t src2, write_only image2d_t dst)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(dst, coord, read_imagef(src1, sampler, coord) + read_imagef(src2, sampler, coord));
}

kernel void krn_sub(const int2 kernelSize, read_only image2d_t src1, read_only image2d_t src2, write_only image2d_t dst)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(dst, coord, read_imagef(src1, sampler, coord) - read_imagef(src2, sampler, coord));
}

kernel void krn_div(const int2 kernelSize, read_only image2d_t dividend, read_only image2d_t divisor,
    write_only image2d_t quotient)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(quotient, coord, clamp(
        native_divide(read_imagef(dividend, sampler, coord), read_imagef(divisor, sampler, coord)),
        (float4)(0.0f, 0.0f, 0.0f, 0.0f),
        (float4)(1.0f, 1.0f, 1.0f, 1.0f)
        )
    );
}

kernel void krn_mul(const int2 kernelSize, read_only image2d_t src1, read_only image2d_t src2, write_only image2d_t dst)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(dst, coord, read_imagef(src1, sampler, coord) * read_imagef(src2, sampler, coord).xxxx);
}

kernel void krn_fill(const int2 kernelSize, const float4 value, write_only image2d_t image)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(image, coord, value);
}

kernel void krn_upsample(const int2 kernelSize, read_only image2d_t small, write_only image2d_t big, const int2 bigMaxCoord)
{
    const int2 smallCoord = (int2)(get_global_id(0), get_global_id(1));
    if(any(smallCoord >= kernelSize))
        return;
    const int2 bigCoord = smallCoord * (int2)(2,2);
    write_imagef(big, min(bigCoord + (int2)(1,0), bigMaxCoord), (float4)(0.0f));
    write_imagef(big, min(bigCoord + (int2)(0,1), bigMaxCoord), (float4)(0.0f));
    write_imagef(big, min(bigCoord + (int2)(1,1), bigMaxCoord), (float4)(0.0f));
    write_imagef(big, bigCoord, read_imagef(small, sampler, smallCoord));
}

kernel void krn_toRgba(const int2 kernelSize, read_only image2d_t src, write_only image2d_t dst)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(dst, coord, clamp(read_imagef(src, sampler, coord),
        (float4)(0.0f, 0.0f, 0.0f, 1.0f),
        (float4)(1.0f, 1.0f, 1.0f, 1.0f)));
}

kernel void krn_copy(const int2 kernelSize, read_only image2d_t src, write_only image2d_t dst)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    write_imagef(dst, coord, read_imagef(src, sampler, coord));
}

kernel void krn_filterGauss(const int2 kernelSize, read_only image2d_t src, write_only image2d_t dst,
    const int8 options, const float4 factor)
/* options:
    01: maxCoord => 'src' size - (1,1)
    23: src coord factor => (1,1) to read all pixels, (2,2) to read even rows/columns only
    45: dst coord factor => (1,1) to write all pixels, (2,2) to write even rows/columns only
    67: direction => (1,0) for horizontal filtering, (0,1) for vertical filtering
*/
// gaussian 1D 5: (0.0625) (0.25) (0.375) (0.25) (0.0625)
// gaussian 1D 7: (0.03125) (0.109375) (0.21875) (0.28125) (0.21875) (0.109375) (0.03125)
{
    const int2 coord = (int2)(get_global_id(0), get_global_id(1));
    if(any(coord >= kernelSize))
        return;
    const int2 srcCoord = coord * options.s23;
    const float4 color =
    // center pixel
        read_imagef(src, sampler, borderCoord(srcCoord, options.s01)) * (float4)(0.375f)
    // direct neighbours
        + read_imagef(src, sampler, borderCoord(srcCoord + (int2)(1,1) * options.s67, options.s01))
            * (float4)(0.25f)
        + read_imagef(src, sampler, borderCoord(srcCoord - (int2)(1,1) * options.s67, options.s01))
            * (float4)(0.25f)
    // 2nd neighbours
        + read_imagef(src, sampler, borderCoord(srcCoord + (int2)(2,2) * options.s67, options.s01))
            * (float4)(0.0625f)
        + read_imagef(src, sampler, borderCoord(srcCoord - (int2)(2,2) * options.s67, options.s01))
            * (float4)(0.0625f);
    write_imagef(dst, coord * options.s45, color * factor);
}
