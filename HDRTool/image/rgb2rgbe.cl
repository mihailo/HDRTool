__kernel void rgb2rgbe(__global const float *image, 
					   __global unsigned int *r, __global unsigned int *g, __global unsigned int *b, __global unsigned int *e, 
					   int height, int width)
{
	// get index into global data array


	
    int y = get_global_id(0);
	int x = get_global_id(1);

	int index = y * get_global_size(1) + x;
	
	

	//int y = get_group_id(0) * 16 + get_local_id(0);
	//int x = get_group_id(1) * 16 + get_local_id(1);
	
	//int index = (get_goup_id(0) * 16 + get_local_id(0)) * 16 + get_group_id(1) * 16 + get_local(1); 

	/*__local float l_Transpose[BLOCK_Y][BLOCK_X + 1];
    const uint    localX = get_local_id(0);
    const uint    localY = BLOCK_SIZE * get_local_id(1);
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCK_X + localX;
    const uint   globalY = get_group_id(1) * BLOCK_Y + localY;*/



    /*if (y >= width || x >= height)
    {   
        return; 
    }*/
    
	float floatR = image[index * 3 + 0];
	float floatG = image[index * 3 + 1];
	float floatB = image[index * 3 + 2];

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
		r[index] = 0;
		g[index] = 0;
		b[index] = 0;
		e[index] = 0;
	}
	else
	{
		int exp;  // exponent
  
		v = frexp(v, &exp) * 256.0 / v;
		r[index] = (unsigned int)(v * floatR);
		g[index] = (unsigned int)(v * floatG);
		b[index] = (unsigned int)(v * floatB);
		e[index] = (unsigned int)(exp + 128);

		//r[y] = y;
		//g[x] = x;
		//b[y * width + x] = 24;
		//e[y * width + x] = 24;
	}
}
