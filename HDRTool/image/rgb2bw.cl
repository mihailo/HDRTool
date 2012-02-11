__kernel void rgb2bw(__global const unsigned char *image, 
					   __global unsigned char *image_bw, 
					   int height, int width)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

    if (y >= height || x >= width)
    {   
        return; 
    }
    
	image_bw[y * width + x] = (image[(y * width + x) * 3 + 0] * 54 
				+ image[(y * width + x) * 3 + 1] * 183
				+ image[(y * width + x) * 3 + 2] * 19) / 256;
}
