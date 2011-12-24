__kernel void rgb2rgbe(__global const float *image, 
					   __global unsigned int *r, __global unsigned int *g, __global unsigned int *b, __global unsigned int *e, 
					   int height, int width)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

    if (y >= height || x >= width)
    {   
        return; 
    }
    
	float floatR = image[(y * width + x) * 3 + 0];
	float floatG = image[(y * width + x) * 3 + 1];
	float floatB = image[(y * width + x) * 3 + 2];

    floatR /= 179;
	floatG /= 179;
	floatB /= 179;
	
	float v = floatR;   // max rgb value
	if(v < floatG)
		v = floatG;
	if(v < floatB)
		v = floatB;
	
	if(v < 1e-32)
	{
		r[y * width + x] = 0;
		g[y * width + x] = 0;
		b[y * width + x] = 0;
		e[y * width + x] = 0;
	}
	else
	{
		int exp;  // exponent
  
		v = frexp(v, &exp) * 256.0 / v;
		r[y * width + x] = (unsigned int)(v * floatR);
		g[y * width + x] = (unsigned int)(v * floatG);
		b[y * width + x] = (unsigned int)(v * floatB);
		e[y * width + x] = (unsigned int)(exp + 128);
	}
}
