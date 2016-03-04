//--------------------------------------------------------------
__kernel void drop2D(write_only image2d_t dstImage, const float2 pos, const float radius, const float ringSize, const float4 color)
{
    int2 coords = (int2)(get_global_id(0), get_global_id(1));
    float2 pnt = (float2)((float)coords.x, (float)coords.y);
    float dist = distance(pnt, pos);
    if (fabs(dist - radius) < ringSize) {
        write_imagef(dstImage, coords, color);
    }
}

//--------------------------------------------------------------
__kernel void ripples2D(read_only image2d_t srcImage, read_only image2d_t intImage, write_only image2d_t dstImage, const float damping)
{
    int2 coords = (int2)(get_global_id(0), get_global_id(1));

    int2 offset[4];
    offset[0] = (int2)(-1,  0);
    offset[1] = (int2)( 1,  0);
    offset[2] = (int2)( 0,  1);
    offset[3] = (int2)( 0, -1);
    
    //  Grab the information arround the active pixel.
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]
    sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    float4 sum = float4(0.0f);
    for (int i = 0; i < 4 ; ++i) {
        sum += read_imagef(srcImage, smp, coords + offset[i]);
    }
    
    //  Make an average and substract the center value.
    sum = (sum / 2.0f) - read_imagef(intImage, smp, coords);
    sum *= damping;

    float4 color = sum;
    write_imagef(dstImage, coords, color);
};

//--------------------------------------------------------------
__kernel void copy2D(read_only image2d_t srcImage, write_only image2d_t dstImage)
{
    int2 coords = (int2)(get_global_id(0), get_global_id(1));
    float4 color = read_imagef(srcImage, coords);
    write_imagef(dstImage, coords, color);
}

//--------------------------------------------------------------
__kernel void drop3D(write_only image3d_t dstImage, const float3 pos, const float radius, const float ringSize, const float4 color)
{
    int4 coords = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);
    float3 pnt = (float3)((float)coords.x, (float)coords.y, (float)coords.z);
    float dist = distance(pnt, pos);
    if (fabs(dist - radius) < ringSize) {
        write_imagef(dstImage, coords, color);
    }
}

//--------------------------------------------------------------
__kernel void ripples3D(read_only image3d_t srcImage, read_only image3d_t intImage, write_only image3d_t dstImage, const float damping)
{
    int4 coords = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);

    int4 offset[6];
    offset[0] = (int4)(-1,  0,  0, 0);
    offset[1] = (int4)( 1,  0,  0, 0);
    offset[2] = (int4)( 0,  1,  0, 0);
    offset[3] = (int4)( 0, -1,  0, 0);
    offset[4] = (int4)( 0,  0, -1, 0);
    offset[5] = (int4)( 0,  0,  1, 0);

    //  Grab the information arround the active pixel.
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]
    sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    float4 sum = float4(0.0f);
    for (int i = 0; i < 6 ; ++i) {
        sum += read_imagef(srcImage, smp, coords + offset[i]);
    }

    //  Make an average and substract the center value.
    sum = (sum / 3.0f) - read_imagef(intImage, smp, coords);
    sum *= damping;

    float4 color = sum;
    write_imagef(dstImage, coords, color);
};

//--------------------------------------------------------------
__kernel void copy3D(read_only image3d_t srcImage, write_only image3d_t dstImage)
{
    int4 coords = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);
    float4 color = read_imagef(srcImage, coords);
    write_imagef(dstImage, coords, color);
}

//--------------------------------------------------------------
__kernel void msa_flipx(read_only image2d_t srcImage, write_only image2d_t dstImage) {                                                                                            
	int i = get_global_id(0);
	int j = get_global_id(1);
	int2 coords1 = (int2)(i, j);
	int2 coords2 = (int2)(get_image_width(srcImage) - i - 1, j);
	sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	float4 color = read_imagef(srcImage, smp, coords1);
	write_imagef(dstImage, coords2, color);
};

//--------------------------------------------------------------
__kernel void msa_flipy(read_only image2d_t srcImage, write_only image2d_t dstImage) {                                                                                            
	int i = get_global_id(0);
	int j = get_global_id(1);
	int2 coords1 = (int2)(i, j);
	int2 coords2 = (int2)(i, get_image_height(srcImage) - j - 1.0f);
	sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	float4 color = read_imagef(srcImage, smp, coords1);
	write_imagef(dstImage, coords2, color);
};

//--------------------------------------------------------------
__kernel void msa_greyscale(read_only image2d_t srcImage, write_only image2d_t dstImage) {                                                                                            
	int2 coords = (int2)(get_global_id(0), get_global_id(1));
	sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	float4 color = read_imagef(srcImage, smp, coords);
	float luminance = 0.3f * color.x + 0.59 * color.y + 0.11 * color.z;
	color = (float4)(luminance, luminance, luminance, 1.0f);
	write_imagef(dstImage, coords, color);                                     
};

//--------------------------------------------------------------
__kernel void msa_invert(read_only image2d_t srcImage, write_only image2d_t dstImage) {                                                                                            
	int2 coords = (int2)(get_global_id(0), get_global_id(1));
	sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	float4 color = read_imagef(srcImage, smp, coords);
	color = (float4)(1.0f, 1.0f, 1.0f, 1.0f) - (float4)(color.xyz, 0.0f);
	write_imagef(dstImage, coords, color);
};


//--------------------------------------------------------------
__kernel void msa_threshold(read_only image2d_t srcImage, write_only image2d_t dstImage, const float thresholdLevel) {
	int2 coords = (int2)(get_global_id(0), get_global_id(1));
	sampler_t smp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	float4 color = read_imagef(srcImage, smp, coords);
	write_imagei(dstImage, coords, isgreaterequal(color, thresholdLevel));   
};

