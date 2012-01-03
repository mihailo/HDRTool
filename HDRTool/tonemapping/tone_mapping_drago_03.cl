


float biasFunc(float b, float x)
{
	return pow(x, b);		// pow(x, log(bias)/log(0.5)
}
float clampq(float v, float min, float max)
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
		pic[y * width * 3 + 3 * x + 0] = (guchar)(clampq(hdr_cuda[y * width * 3 + 3 * x + 0] * scale, 0.0f, 1.0f) * 255.0f);
		pic[y * width * 3 + 3 * x + 1] = (guchar)(clampq(hdr_cuda[y * width * 3 + 3 * x + 1] * scale, 0.0f, 1.0f) * 255.0f);
		pic[y * width * 3 + 3 * x + 2] = (guchar)(clampq(hdr_cuda[y * width * 3 + 3 * x + 2] * scale, 0.0f, 1.0f) * 255.0f);
	}
}

void tmo_drago03(unsigned int width, unsigned int height, const float* hdr, guchar* img_drago03, float maxLum, float avLum, float bias)
{
	/*const float LOG05 = -0.693147f; // log(0.5)
	maxLum /= avLum;				// normalize maximum luminance by average luminance

	float divider = log10(maxLum+1.0f);
	float biasP = log(bias)/LOG05;


	// Normal tone mapping of every pixel
	int i, j;
	for(j = 0; j < height; j++)
		for(i = 0; i < width; i++)
	    {
			float Yw = hdr[j * width * 3 + 3 * i + 1] / avLum;
			float interpol = log (2.0f + biasFunc(biasP, Yw / maxLum) * 8.0f);
			img_drago03[j * width * 3 + 3 * i + 1] = ( log(Yw+1.0f)/interpol ) / divider;
	    }


	for(j = 0; j < height; j++)
		for(i = 0; i < width; i++)
	      {
			float scale = img_drago03[j * width * 3 + 3 * i + 1] / hdr[j * width * 3 + 3 * i + 1];
	        img_drago03[j * width * 3 + 3 * i + 0] = hdr[j * width * 3 + 3 * i + 0] * scale;
	        img_drago03[j * width * 3 + 3 * i + 1] = hdr[j * width * 3 + 3 * i + 1] * scale;
	        img_drago03[j * width * 3 + 3 * i + 2] = hdr[j * width * 3 + 3 * i + 2] * scale;
	        //printf("%f ",img_drago03[j * width * 3 + 3 * i + 0]);
	      }*/
	float normMaxLum = maxLum / avLum; // normalize maximum luminance by average luminance
	const float LOG05 = -0.693147f; // log(0.5)

	float divider = log10(normMaxLum+1.0f);
	float biasP = log(bias)/LOG05;
	printf("divider = %f biasP = %f \n", divider, biasP);
	creatStopwatch();
	int timer_id = startStopwatch();

	float *hdr_cuda;
	guchar *tmo_drago03_cuda;
	size_t size_hdr = width * height * 3 * sizeof(float);
	size_t size_drago03 = width * height * 3 * sizeof(guchar);
	cudaMalloc((void **)&hdr_cuda, size_hdr);
	//cudaMalloc((void **)&tmo_drago03_cuda, size_hdr);
	cudaMalloc((void **)&tmo_drago03_cuda, size_drago03);
	cudaMemcpy(hdr_cuda, hdr, size_hdr, cudaMemcpyHostToDevice);

	float time_alloc = getTime(timer_id);
	int timer_id_calc = startStopwatch();

	// Invoke kernel
	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
	dim3 dimGrid((width + BLOCK_SIZE - 1) / dimBlock.x, (height + BLOCK_SIZE - 1) / dimBlock.y);
	printf("dimenzije u blokovima: %d, %d\n", (width + BLOCK_SIZE - 1) / dimBlock.x, (height + BLOCK_SIZE - 1) / dimBlock.y);
	tmo_drago03_pixel<<<dimGrid, dimBlock>>>(width, height, hdr_cuda, tmo_drago03_cuda, avLum, normMaxLum, biasP, divider);

	float time_calc = getTime(timer_id_calc);
	int timer_id_cpy = startStopwatch();

	cudaMemcpy(img_drago03, tmo_drago03_cuda, size_drago03, cudaMemcpyDeviceToHost);

	float time_cpy = getTime(timer_id_cpy);
	float time_total = getTime(timer_id);
	addTime("allocate memory on Device and copy data to device: ", time_alloc);
	addTime("tone mapping drago03: ", time_calc);
	addTime("copy data from Device to Host: ", time_cpy);
	addTime("total time: ", time_total);
	destroyStopwatch();

	cudaFree(hdr_cuda);
	cudaFree(tmo_drago03_cuda);
}

/*__global__ void calculateLuminancePixel(const unsigned int width, const unsigned int height, float *hdr_cuda, float *avLumArray_cuda)
{
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	if(y < height && x < width)
	{
		avLumArray_cuda[y * width + x] = log( hdr_cuda[y * width * 3 + 3 * x + 1] + 1e-4 );
	}
}*/

void calculateLuminance(const unsigned int width, const unsigned int height, float *hdr, float *avLum, float *maxLum)
{
	*avLum = 0.0f;
  	*maxLum = 0.0f;

  	/*float *hdr_cuda;
  	size_t size_hdr_cuda = width * height * 3 * sizeof(float);
  	cudaMalloc((void **)&hdr_cuda, size_hdr_cuda);
  	cudaMemcpy(hdr_cuda, hdr, size_hdr_cuda, cudaMemcpyHostToDevice);

  	float *avLumArray, *avLumArray_cuda;
  	avLumArray = (float *)malloc(width * height * sizeof(float));
  	size_t size_float = width * height * sizeof(float);
  	cudaMalloc((void **)&avLumArray_cuda, size_float);

  	// Invoke kernel
  	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
  	dim3 dimGrid((width + BLOCK_SIZE - 1) / dimBlock.x, (height + BLOCK_SIZE - 1) / dimBlock.y);
  	printf("dimenzije u blokovima: %d, %d\n", (width + BLOCK_SIZE - 1) / dimBlock.x, (height + BLOCK_SIZE - 1) / dimBlock.y);
  	calculateLuminancePixel<<<dimGrid, dimBlock>>>(width, height, hdr_cuda, avLumArray_cuda);

	cudaMemcpy(avLumArray, avLumArray_cuda, size_float, cudaMemcpyDeviceToHost);

	int i, j;
	for(i = 0; i < height; i++)
		for(j = 0; j < width; j++)
		{
			*avLum += avLumArray[i * width + j];
			//printf("%f ", avLumArray[i * width + j]);
			*maxLum = ( hdr[i * width * 3 + 3 * j + 1] > *maxLum ) ? hdr[i * width * 3 + 3 * j + 1] : *maxLum ;
		}

  	float size = width * height;
  	*avLum =exp(*avLum / size);
  	printf("avLum = %f maxLum = %f\n", *avLum, *maxLum);

  	cudaFree(hdr_cuda);
  	cudaFree(avLumArray_cuda);*/

  	creatStopwatch();
  	int timer_id = startStopwatch();

  	float *hdr_cuda;
  	size_t size_hdr_cuda = width * height * 3 * sizeof(float);
  	cudaMalloc((void **)&hdr_cuda, size_hdr_cuda);
  	cudaMemcpy(hdr_cuda, hdr, size_hdr_cuda, cudaMemcpyHostToDevice);

  	float *avLumArray, *avLumArray_cuda;
  	float *maxLumArray, *maxLumArray_cuda;
  	int size_x = (width + BLOCK_SIZE - 1) / BLOCK_SIZE;
  	int size_y = (height + BLOCK_SIZE - 1) / BLOCK_SIZE;

  	avLumArray = (float *)malloc(size_x * size_y * sizeof(float));
  	maxLumArray = (float *)malloc(size_x * size_y * sizeof(float));
  	size_t size_floatxy = size_x * size_y * sizeof(float);
	int i;
  	for(i = 0; i < size_x * size_y; i++)
  		avLumArray[i] = 0;
  	cudaMalloc((void **)&avLumArray_cuda, size_floatxy);
  	cudaMalloc((void **)&maxLumArray_cuda, size_floatxy);
  	cudaMemcpy(avLumArray_cuda, avLumArray, size_floatxy, cudaMemcpyHostToDevice);

  	float time_alloc = getTime(timer_id);
  	int timer_id_calc = startStopwatch();

  	// Invoke kernel
  	dim3 dimBlocka(BLOCK_SIZE, BLOCK_SIZE);
  	dim3 dimGrida((width + BLOCK_SIZE - 1) / dimBlocka.x, (height + BLOCK_SIZE - 1) / dimBlocka.y);
  	printf("dimenzije u blokovima: %d, %d\n", (width + BLOCK_SIZE - 1) / dimBlocka.x, (height + BLOCK_SIZE - 1) / dimBlocka.y);
  	reduction_calculateLuminancePixel<<<dimGrida, dimBlocka>>>(width, height, hdr_cuda, size_x,avLumArray_cuda, maxLumArray_cuda);

  	float time_calc = getTime(timer_id_calc);
  	int timer_id_cpy = startStopwatch();

  	cudaMemcpy(avLumArray, avLumArray_cuda, size_floatxy, cudaMemcpyDeviceToHost);
  	cudaMemcpy(maxLumArray, maxLumArray_cuda, size_floatxy, cudaMemcpyDeviceToHost);

  	float time_cpy = getTime(timer_id_cpy);
  	int timer_id_cpu = startStopwatch();


  	*avLum = 0.0f;
  	*maxLum = 0.0f;
  	for(i = 0; i < (size_x * size_y); i++)
  	{
  		*avLum += avLumArray[i];
  		*maxLum = ( maxLumArray[i] > *maxLum ) ? maxLumArray[i] : *maxLum ;
  	}

  	float size = width * height;
  	*avLum =exp(*avLum / size);

  	float time_cpu = getTime(timer_id_cpu);
  	float time_total = getTime(timer_id);

  	//float time_calc = getTime(timer_id_calc);
  	//int timer_id_cpy = startStopwatch();
  	//float time_cpy = getTime(timer_id_cpy);

  	addTime("allocate memory on Device and copy data to device: ", time_alloc);
  	addTime("calc time on cuda: ", time_calc);
  	addTime("calc time on cpu: ", time_cpu);
  	addTime("copy data from Device to Host: ", time_cpy);
  	addTime("total time: ", time_total);
  	destroyStopwatch();


  	//printf("avLum = %f maxLum = %f\n", *avLum, *maxLum);
}
 