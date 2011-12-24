__kernel void rgbe2rgb(__global float *image, 
					   __global const unsigned int *r, __global const unsigned int *g, __global const unsigned int *b, __global const unsigned int *e, 
					   int height, int width, float exposure)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

    if (y >= height || x >= width)
    {   
        return; 
    }
    
	if(e[y * width + x] != 0)     // a non-zero pixel
	{
		int exp = e[y * width + x] - (int)(128 + 8);
		//TODO it was double ???
		float f = ldexp( 1.0, exp ) * 179.0 / exposure;
		
		image[(y * width + x) * 3 + 0] = (float)(r[y * width + x] * f);
		image[(y * width + x) * 3 + 1] = (float)(g[y * width + x] * f);
		image[(y * width + x) * 3 + 2] = (float)(b[y * width + x] * f);
		
		/*
		//bez ovoga za tmo_drago03
		image[(y * width + x) * 3 + 0] = clamp( image[(y * width + x) * 3 + 0], 0, 255 );
		image[(y * width + x) * 3 + 0] = clamp( image[(y * width + x) * 3 + 0], 0, 255 );
		image[(y * width + x) * 3 + 0] = clamp( image[(y * width + x) * 3 + 0], 0, 255 );
		*/
	}
	else
	{
		image[(y * width + x) * 3 + 0] = 0;
		image[(y * width + x) * 3 + 1] = 0;
		image[(y * width + x) * 3 + 2] = 0;
	}
}
