long sumimage(unsigned char *diff)
{
  	
	
	
	return sum;
}
__kernel void align(__global unsigned char *image1, __global unsigned char *image2,
					__global unsigned char *threshold1, __global unsigned char *threshold2,
					__global unsigned char *mask, __global unsigned char *mask,
					__global unsigned char *diff,
					int median1, int median2,
					__global int x_shift, __global int y_shift, 
					int shift_bits, int noise,
					unsigned int const height, unsigned int const width)
{
	int y = get_global_id(0);
	int x = get_global_id(1);
	
	if (y >= height || x >= width)
    {   
        //sync after calculations of thresholds and masks
		barrier(CL_GLOBAL_MEM_FENCE);

		for(int l = 0; l < shift_bits; l++)
		{
			//sync after calculations diff image
			barrier(CL_GLOBAL_MEM_FENCE);
		}
	
		return;
    }
	
	
	
	int temp_level = 0;
	for(int l = 0; l < shift_bits; l++)
	{
		temp_level = pow(2,l);
		if(y % temp_level == 0 && x % temp_level == 0)
		{
			//scale
			

			
			threshold1[y * width + x] = image1[y * width + x] < median1 ? 0 : 1;
			threshold2[y * width + x] = image2[y * width + x] < median2 ? 0 : 1;
	
			mask1[y * width + x] = (image1[y * width + x] > (median1 - noise)) && (image1[y * width + x] < (median1 + noise)) ? 0 : 1;
			mask2[y * width + x] = (image1[y * width + x] > (median2 - noise)) && (image1[y * width + x] < (median2 + noise)) ? 0 : 1;
	
			//sync after calculations of thresholds and masks
			barrier(CL_GLOBAL_MEM_FENCE);
			
			
			
			
			
			
			
			
			
			
			
			
			int minerr = width * height; //depends on level
			diff[y * width + x] = 0; //fill(0)
			for(int i = -1; i <= 1; i++) 
			{
				int dy = y + j;
				if(dy < 0) continue;
				if(dy >= height) break;
				for(int j = -1; j <= 1; j++) 
				{
					int dx = x + i;
					if(dx >= width) break;
					if(dy >= 0 && dy < height && dx >= 0 && dx <= width)
					{
						unsigned char pix = (threshold1[y * width + x] xor threshold2[dy * width + dx]) and mask1[y * width + x] and mask2[dy * width + dx]; 
						diff[y * width + x] = pix;
					}
				}
			}
		
			//sync after calculations diff image
			barrier(CL_GLOBAL_MEM_FENCE);

			long err = sumimage(diff);
			if( err < minerr ) 
			{
				minerr = err;
				shift_x = dx;
				shift_y = dy;
			}
		}
		else
		{
		}
	}
	
	
}