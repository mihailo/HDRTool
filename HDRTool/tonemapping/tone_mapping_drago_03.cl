float biasFunc(float b, float x)
{
	return pow(x, b);		// pow(x, log(bias)/log(0.5)
}
float clamp(float v, float min, float max)
{
	if(v < min) v = min;
	if(v > max) v = max;
	return v;
}
__kernel__ void tone_mapping_drago03(unsigned int width, unsigned int height, 
									 __global const float* image, 
									 __global unsigned int* pic, 
									 float avLum, float normMaxLum, float biasP, float divider)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

	if(y < height && x < width)
	{
		float Yw = image[y * width * 3 + 3 * x + 1] / avLum;
		float interpol = log (2.0f + biasFunc(biasP, Yw / normMaxLum) * 8.0f);
		float yg = ( log(Yw+1.0f)/interpol ) / divider;

		float scale = yg / image[y * width * 3 + 3 * x + 1];
		pic[y * width * 3 + 3 * x + 0] = (guchar)(clamp(hdr_cuda[y * width * 3 + 3 * x + 0] * scale, 0.0f, 1.0f) * 255.0f);
		pic[y * width * 3 + 3 * x + 1] = (guchar)(clamp(hdr_cuda[y * width * 3 + 3 * x + 1] * scale, 0.0f, 1.0f) * 255.0f);
		pic[y * width * 3 + 3 * x + 2] = (guchar)(clamp(hdr_cuda[y * width * 3 + 3 * x + 2] * scale, 0.0f, 1.0f) * 255.0f);
	}
}