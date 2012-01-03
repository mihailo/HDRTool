__kernel void calculate_luminance_pixel(const unsigned int width, const unsigned int height, __global float *cl_floatImage, 
										__global float *avLumArray, __global float *maxLumArray,
										__local float *avLumBlock[BLOCK_SIZE][BLOCK_SIZE], 
										__local float *maxLumBlock[BLOCK_SIZE][BLOCK_SIZE])
{
	int y = get_global_id(0);
	int x = get_global_id(1);

	if(y < height && x < width)
	{
		avLumBlock[get_local_id(0)][get_local_id(1)] = log( image[y * width * 3 + 3 * x + 1] + 1e-4 );
		maxLumBlock[get_local_id(0)][get_local_id(1)] = image[y * width * 3 + 3 * x + 1];
	}
	else
	{
		avLumBlock[get_local_id(0)][get_local_id(1)] = 0;
		maxLumBlock[get_local_id(0)][get_local_id(1)] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(1) == 0)
	{
		int i;
		for(i = 1; i < BLOCK_SIZE; i++)
		{
			avLumBlock[get_local_id(0)][[get_local_id(1)] += avLumBlock[get_local_id(0)][i];
			if(maxLumBlock[get_local_id(0)][[get_local_id(1)] < maxLumBlock[get_local_id(0)][i])
				maxLumBlock[get_local_id(0)][[get_local_id(1)] = maxLumBlock[get_local_id(0)][i];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(1) == 0 && get_local_id(0) == 0)
	{
		int i;
		for(i = 1; i < BLOCK_SIZE; i++)
		{
			avLumBlock[get_local_id(0)][get_local_id(1)] += avLumBlock[i][get_local_id(1)];
			if(maxLumBlock[get_local_id(0)][get_local_id(1)] < maxLumBlock[i][get_local_id(1)])
				maxLumBlock[get_local_id(0)][get_local_id(1)] = maxLumBlock[i][get_local_id(1)];
		}

		//size_x num of block x
		avLumArray_cuda[get_local_id(0) * size_x + get_local_id(1)] = avLumBlock[get_local_id(0)][get_local_id(1)];
		maxLumArray_cuda[get_local_id(0) * size_x + get_local_id(1)] = maxLumBlock[get_local_id(0)][get_local_id(1)];
	}
}