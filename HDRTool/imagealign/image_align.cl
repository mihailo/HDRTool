//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable

__kernel void align(__global unsigned char *image1,		//0 
					__global unsigned char *image2,		//1
					__global unsigned char *threshold1,	//2
					__global unsigned char *threshold2,	//3
					__global unsigned char *mask1,		//4
					__global unsigned char *mask2,		//5
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
	
	int minerr = width * height; 
	int shift_x = 10;// = -1;
	int shift_y = 10;// = -1;
	
	if(get_local_id(0) == 0 && get_local_id(1) == 0)
	{
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
	
	if(y < height && x < width)
	{
		threshold1[y * width + x] = image1[y * width + x] < median1 ? 0 : 1;
		threshold2[y * width + x] = image2[y * width + x] < median2 ? 0 : 1;
	
		mask1[y * width + x] = (image1[y * width + x] > (median1 - noise)) && (image1[y * width + x] < (median1 + noise)) ? 0 : 1;
		mask2[y * width + x] = (image2[y * width + x] > (median2 - noise)) && (image2[y * width + x] < (median2 + noise)) ? 0 : 1;
	}
	
	//sync after calculations of thresholds and masks, becouse shifting of image2
	barrier(CLK_GLOBAL_MEM_FENCE);
		
	if(y < height && x < width)
	{
		for(int i = -1; i <= 1; i++) 
		{
			int dy = y + i;
			if(dy < 0) continue;
			if(dy >= height) break;
			for(int j = -1; j <= 1; j++) 
			{
				int dx = x + j;
				if(dx >= width) break;
				if(dx >= 0)
				{
					unsigned char pix = (threshold1[y * width + x] ^ threshold2[dy * width + dx]) 
										&& mask1[y * width + x] && mask2[dy * width + dx]; 
					if(pix == 1)
					{
						atom_inc(&local_error[(i + 1) * 3 + j + 1]);
					}
				}
			}
		}
	}
		
	//sync after calculations all 9 diff images
	barrier(CLK_GLOBAL_MEM_FENCE);

	if(get_local_id(0) == 0 && get_local_id(1) == 0)
	{
		for(int k = 0; k < 9; k++)
		{
			atom_add(&errors[k], local_error[k]);
		}
	}
		
	//sync after calculations all 9 diff images
	barrier(CLK_GLOBAL_MEM_FENCE);	
	
	if(y == 0 && x == 0)
	{
				
		for(int i = -1; i <= 1; i++)
		{
			for(int j = -1; j <= 1; j++)
			{
				if( errors[(i + 1) * 3 + j + 1] < minerr ) 
				{
					minerr = errors[(i + 1) * 3 + j + 1];
					shift_x = j;
					shift_y = i;
				}
			}
		}
		x_shift[0] = shift_x;//l
		y_shift[0] = shift_y;//l
	}
				
	//sync after level
	//barrier(CLK_GLOBAL_MEM_FENCE);
}