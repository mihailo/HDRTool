//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable

__kernel void align(__global unsigned char *image1,		//0 
					__global unsigned char *image2,		//1
					__local unsigned char *threshold1,	//2
					__local unsigned char *threshold2,	//3
					__local unsigned char *mask1,		//4
					__local unsigned char *mask2,		//5
					__local int *local_error,			//6
					__global int *errors,				//7
					int median1,						//8
					int median2,						//9
					__global int *x_shift,				//10
					__global int *y_shift,				//11
					int shift_bits,						//12
					int noise,							//13
					unsigned int const height,			//14
					unsigned int const width)			//15
{
	int y = get_global_id(0);
	int x = get_global_id(1);
	
	int local_width = get_local_size(1) + 2;
	int local_y = get_local_id(0) + 1;
	int local_x = get_local_id(1) + 1;

	if(get_local_id(0) == 0 && get_local_id(1) == 0)
	{
		for(int k = 0; k < get_local_size(0) + 2; k++)
		{
			int first_row = 0 * local_width + k;
			threshold1[first_row] = threshold2[first_row] = mask1[first_row] = mask2[first_row] = 0;
			int last_row = (get_local_size(0) + 1) * local_width + k;
			threshold1[last_row] = threshold2[last_row] = mask1[last_row] = mask2[last_row] = 0;
			int first_col = k * local_width + 0;
			threshold1[first_col] = threshold2[first_col] = mask1[first_col] = mask2[first_col] = 0;
			int last_col = k * local_width + get_local_size(1) + 1;
			threshold1[last_col] = threshold2[last_col] = mask1[last_col] = mask2[last_col] = 0;
		}
		threshold1[0] = threshold2[0] = mask1[0] = mask2[0] = 0;
		threshold1[get_local_size(1) + 1] = threshold2[get_local_size(1) + 1] = mask1[get_local_size(1) + 1] = mask2[get_local_size(1) + 1] = 0;
		threshold1[(get_local_size(0) + 1) * local_width] = threshold2[(get_local_size(0) + 1) * local_width] = mask1[(get_local_size(0) + 1) * local_width] = mask2[(get_local_size(0) + 1) * local_width] = 0;
		threshold1[(get_local_size(0) + 1) * local_width + get_local_size(1) + 1] = threshold2[(get_local_size(0) + 1) * local_width + get_local_size(1) + 1] = mask1[(get_local_size(0) + 1) * local_width + get_local_size(1) + 1] = mask2[(get_local_size(0) + 1) * local_width + get_local_size(1) + 1] = 0;
		for(int k = 0; k < 9; k++)
		{
					local_error[k] = 0;
		}
	}

	if(x == 0 && y == 0)
	{
		for(int k = 0; k < 9; k++)
		{
					errors[k] = 0;
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	barrier(CLK_GLOBAL_MEM_FENCE); //local

	if(y < height && x < width)
	{
		int local_index = local_y * local_width + local_x;
		int global_index = y * width + x;
		
		threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
		threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
		mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
		mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;

		if(get_local_id(0) == 0 && y != 0)
		{
			// -1 row
			local_index = 0 * local_width + local_x;
			global_index = (y - 1) * width + x;

			threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
			threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
			mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
			mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
		}

		if(get_local_id(0) == (get_local_size(0) - 1) && y != (height - 1))
		{
			// +1 row
			local_index = (get_local_size(0) + 1) * local_width + local_x;
			global_index  = (y + 1) * width + x;

			threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
			threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
			mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
			mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
		}

		if(get_local_id(1) == 0 && x != 0)
		{
			// -1 col
			local_index = local_y * local_width + 0;
			global_index = y * width + (x - 1);

			threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
			threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
			mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
			mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
		}

		if(get_local_id(1) == (get_local_size(1) - 1) && x != (width - 1))
		{
			// +1 col
			local_index = local_y * local_width + (get_local_size(1) + 1);
			global_index = y * width + (x + 1);

			threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
			threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
			mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
			mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
		}
		
		if(get_local_id(0) == 0 && get_local_id(1) == 0)
		{
			//corners
			if(x != 0 && y != 0)
			{
				local_index = 0;
				global_index = (y - 1) * width + (x - 1);
				
				threshold1[0] = image1[global_index] < median1 ? 0 : 1;
				threshold2[0] = image2[global_index] < median2 ? 0 : 1;
	
				mask1[0] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
				mask2[0] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
			}
			if(x != (width - 1) && y != 0)
			{
				local_index = get_local_size(1) + 1;
				global_index = (y - 1) * width + (x + 1);
				
				threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
				threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
				mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
				mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
			}
			if(x != 0 && y != (height - 1))
			{
				local_index = (get_local_size(0) + 1) * local_width;
				global_index = (y + 1) * width + (x - 1);

				threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
				threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
				mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
				mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
			}
			if(x != (width - 1) && y != (height - 1))
			{
				local_index = (get_local_size(0) + 1) * local_width + get_local_size(1) + 1;
				global_index = (y + 1) * width + (x + 1);
				
				threshold1[local_index] = image1[global_index] < median1 ? 0 : 1;
				threshold2[local_index] = image2[global_index] < median2 ? 0 : 1;
	
				mask1[local_index] = (image1[global_index] > (median1 - noise)) && (image1[global_index] < (median1 + noise)) ? 0 : 1;
				mask2[local_index] = (image2[global_index] > (median2 - noise)) && (image2[global_index] < (median2 + noise)) ? 0 : 1;
			}
		}
	}
	
	//sync after calculations of thresholds and masks, becouse shifting of image2
	barrier(CLK_LOCAL_MEM_FENCE); //local
	
	if(y < height && x < width)
	{
		for(int i = -1; i <= 1; i++) 
		{
			int dy = local_y + i;
			for(int j = -1; j <= 1; j++) 
			{
				int dx = local_x + j;
				unsigned char pix = (threshold1[local_y * local_width + local_x] ^ threshold2[dy * local_width + dx]) 
									&& mask1[local_y * local_width + local_x] && mask2[dy * local_width + dx]; 
				if(pix == 1)
				{
					atom_inc(&local_error[(i + 1) * 3 + j + 1]);
				}
			}
		}
	}
		
	//sync after calculations all 9 diff images
	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(0) == 0 && get_local_id(1) == 0)
	{
		for(int k = 0; k < 9; k++)
		{
			atom_add(&errors[k], local_error[k]);
		}
	}
}
