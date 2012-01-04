#define BLOCK_SIZE 16
__kernel void calculate_luminance_pixel(const unsigned int width, const unsigned int height, __global float *cl_floatImage, 
										__global float *avLumArray, __global float *maxLumArray,
										__local float *avLumBlock, 
										__local float *maxLumBlock)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

	if(y < height && x < width)
	{
		avLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = log( cl_floatImage[y * width * 3 + 3 * x + 1] + 1e-4 );
		maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = cl_floatImage[y * width * 3 + 3 * x + 1];
	}
	else
	{
		avLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = 0;
		maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(1) == 0)
	{
		int i;
		for(i = 1; i < BLOCK_SIZE; i++)
		{
			avLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] += avLumBlock[get_local_id(0) * BLOCK_SIZE + i];
			if(maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] < maxLumBlock[get_local_id(0) * BLOCK_SIZE + i])
				maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = maxLumBlock[get_local_id(0) * BLOCK_SIZE + i];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(1) == 0 && get_local_id(0) == 0)
	{
		int i;
		for(i = 1; i < BLOCK_SIZE; i++)
		{
			avLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] += avLumBlock[i * BLOCK_SIZE + get_local_id(1)];
			if(maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] < maxLumBlock[i * BLOCK_SIZE + get_local_id(1)])
				maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)] = maxLumBlock[i * BLOCK_SIZE + get_local_id(1)];
		}

		//size_x num of block x
		unsigned int block_y = get_global_id(0) / get_local_size(0);
		unsigned int block_x = get_global_id(1) / get_local_size(1);
		unsigned int block_x_dim = get_global_size(1) / get_local_size(1);
		avLumArray[block_y * block_x_dim + block_x] = avLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)];
		maxLumArray[block_y * block_x_dim + block_x] = maxLumBlock[get_local_id(0) * BLOCK_SIZE + get_local_id(1)];
	}
}