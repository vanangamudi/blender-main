/// This file contains all opencl kernels for node-operation implementations 

// Global SAMPLERS
const sampler_t SAMPLER_NEAREST      = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__constant const int2 zero = {0,0};

// KERNEL --- BOKEH BLUR ---
__kernel void bokehBlurKernel(__global __read_only image2d_t boundingBox, __global __read_only image2d_t inputImage, 
                              __global __read_only image2d_t bokehImage, __global __write_only image2d_t output, 
                              int2 offsetInput, int2 offsetOutput, int radius, int step, int2 dimension, int2 offset) 
{
	int2 coords = {get_global_id(0), get_global_id(1)}; 
	coords += offset;
	float tempBoundingBox;
	float4 color = {0.0f,0.0f,0.0f,0.0f};
	float4 multiplyer = {0.0f,0.0f,0.0f,0.0f};
	float4 bokeh;
	const float radius2 = radius*2.0f;
	const int2 realCoordinate = coords + offsetOutput;

	tempBoundingBox = read_imagef(boundingBox, SAMPLER_NEAREST, coords).s0;

	if (tempBoundingBox > 0.0f) {
		const int2 bokehImageDim = get_image_dim(bokehImage);
		const int2 bokehImageCenter = bokehImageDim/2;
		const int2 minXY = max(realCoordinate - radius, zero);
		const int2 maxXY = min(realCoordinate + radius, dimension);
		int nx, ny;
		
		float2 uv;
		int2 inputXy;
		
		for (ny = minXY.y, inputXy.y = ny - offsetInput.y ; ny < maxXY.y ; ny +=step, inputXy.y+=step) {
			uv.y = ((realCoordinate.y-ny)/radius2)*bokehImageDim.y+bokehImageCenter.y;
			
			for (nx = minXY.x, inputXy.x = nx - offsetInput.x; nx < maxXY.x ; nx +=step, inputXy.x+=step) {
				uv.x = ((realCoordinate.x-nx)/radius2)*bokehImageDim.x+bokehImageCenter.x;
				bokeh = read_imagef(bokehImage, SAMPLER_NEAREST, uv);
				color += bokeh * read_imagef(inputImage, SAMPLER_NEAREST, inputXy);
				multiplyer += bokeh;
			}
		}
		color /= multiplyer;
		
	} else {
		int2 imageCoordinates = realCoordinate - offsetInput;
		color = read_imagef(inputImage, SAMPLER_NEAREST, imageCoordinates);
	}
	
	write_imagef(output, coords, color);
}
