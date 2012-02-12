__kernel void align(__global unsigned char *image1, __global unsigned char *image2,
					__global unsigned char *threshold1, __global unsigned char *threshold2,
					__global unsigned char *mask1, __global unsigned char *mask2,
					__global unsigned char *diff,
					int median1, int median2,
					__global int *x_shift, __global int *y_shift, 
					int shift_bits, int noise,
					unsigned int const height, unsigned int const width)
{
	int y = get_global_id(0);
	int x = get_global_id(1);
	
	if (y < height && x < width)
    {   
		
    }
	
	
	
	int temp_level = 0;
	for(int l = 0; l < shift_bits; l++)
	{
		temp_level = (int)pown(2.0f, l);
		if(y % temp_level == 0 && x % temp_level == 0)
		{
			//scale
			if(temp_level > 1)
			{
				for(int i = 0; i < temp_level; i++)
				{
					for(int j = 0; j < temp_level; j++)
					{
						image1[y * width + x] += image1[(y + i) * width + x + j];
					}
				}
				image1[y * width + x] = image1[y * width + x] / temp_level / temp_level; 
			}

			
			threshold1[y * width + x] = image1[y * width + x] < median1 ? 0 : 1;
			threshold2[y * width + x] = image2[y * width + x] < median2 ? 0 : 1;
	
			mask1[y * width + x] = (image1[y * width + x] > (median1 - noise)) && (image1[y * width + x] < (median1 + noise)) ? 0 : 1;
			mask2[y * width + x] = (image1[y * width + x] > (median2 - noise)) && (image1[y * width + x] < (median2 + noise)) ? 0 : 1;
	
			//sync after calculations of thresholds and masks, becouse shifting of image2
			barrier(CLK_GLOBAL_MEM_FENCE);
			
			int temp_width = width / temp_level;
			int temp_height = height / temp_level;
			int minerr = temp_width * temp_height; //depends on level
			int shift_x = -1;
			int shift_y = -1;
			diff[y * width + x] = 0; //fill(0)

			for(int i = -1; i <= 1; i++) 
			{
				int dy = y + i;
				if(dy < 0) continue;
				if(dy >= height) break;
				for(int j = -1; j <= 1; j++) 
				{
					int dx = x + j;
					if(dx >= width) break;
					if(dy >= 0 && dy < height && dx >= 0 && dx <= width)
					{
						unsigned char pix = (threshold1[y * width + x] xor threshold2[dy * width + dx]) and mask1[y * width + x] and mask2[dy * width + dx]; 
						diff[y * width + x] = pix;
					}


					//sync after calculations diff image
					barrier(CLK_GLOBAL_MEM_FENCE);

					//sum_error
					if(x == 0)
					{
						for(int i = 1; i < width; i++)
						{
							diff[y * width + 0] += diff[y * width + i];
						}
					}

					//sunch after calculations of errors in colums
					barrier(CLK_GLOBAL_MEM_FENCE);

					if(x == 0 && y == 0)
					{
						long err = 0;
						for(int i = 0; i < height; i++)
						{
							err += diff[i * width + 0];
						}
						if( err < minerr ) 
						{
							minerr = err;
							shift_x = dx;
							shift_y = dy;
						}
					}
				}
			}
			
			if(x == 0 && y == 0)
			{
				x_shift[l] = x_shift;
				y_shift[l] = y_shift;
			}
			//sunch at end of one level
			barrier(CLK_GLOBAL_MEM_FENCE);
		}
		else
		{
			//synch all another threads
		}
	}
	
	
}