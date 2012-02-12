#include "LuminancePixel.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"

LuminancePixel::LuminancePixel()
{
	core = new OpenCLCore(this, "tonemapping/luminance_pixel.cl", "calculate_luminance_pixel");
}

LuminancePixel::~LuminancePixel()
{
	delete core;
}

void LuminancePixel::calculate_luminance_pixel(Image<float> *img, 
					 float *avLum, float *maxLum)
{
	image = img;
	avLuminance = avLum;
	maxLuminance = maxLum;

	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	
	//round values on upper value
	logFile("%d %d \n", image->getHeight(), image->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, image->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, image->getWidth());

	core->runComputeUnit();
}

void LuminancePixel::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * 3;
	cl_floatImage = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr1);

	unsigned int size2DBlocks = sizeof(cl_float) * roundUp(BLOCK_SIZE, image->getHeight()) * roundUp(BLOCK_SIZE, image->getWidth()) / BLOCK_SIZE / BLOCK_SIZE;
	cl_avLumArray = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size2DBlocks, NULL, &ciErr2);
	avLumArray = new float[roundUp(BLOCK_SIZE, image->getHeight()) * roundUp(BLOCK_SIZE, image->getWidth()) / BLOCK_SIZE / BLOCK_SIZE];
    ciErr1 |= ciErr2;
	
	cl_maxLumArray = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size2DBlocks, NULL, &ciErr2);
    maxLumArray = new float[roundUp(BLOCK_SIZE, image->getHeight()) * roundUp(BLOCK_SIZE, image->getWidth()) / BLOCK_SIZE / BLOCK_SIZE];
	ciErr1 |= ciErr2;
    
	logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void LuminancePixel::setInputDataToOpenCLMemory()
{
	int height = image->getHeight();
	int width = image->getWidth();

	cl_int ciErr1;				
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, 
		sizeof(cl_int), (void*)&width);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, 
		sizeof(cl_int), (void*)&height);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, 
		sizeof(cl_mem), (void*)&cl_floatImage);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, 
		sizeof(cl_mem), (void*)&cl_avLumArray);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_mem), (void*)&cl_maxLumArray);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 5, 
		 sizeof(float) * BLOCK_SIZE *BLOCK_SIZE, 0);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 6, 
		 sizeof(float) * BLOCK_SIZE *BLOCK_SIZE, 0);
    
    logFile("clSetKernelArg 0 - 6...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clSetKernelArg, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_floatImage, CL_TRUE, 0, 
		size, image->getImage(), 0, NULL, NULL);
    logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void LuminancePixel::getDataFromOpenCLMemory()
{
	unsigned int size2DBlocks = sizeof(cl_float) * roundUp(BLOCK_SIZE, image->getHeight()) * roundUp(BLOCK_SIZE, image->getWidth())
		/ BLOCK_SIZE / BLOCK_SIZE;
	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_avLumArray, CL_TRUE, 0, 
		size2DBlocks, avLumArray, 0, NULL, NULL);
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_maxLumArray, CL_TRUE, 0, 
		size2DBlocks, maxLumArray, 0, NULL, NULL);
    
	logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
	final_calculation();
}

void LuminancePixel::final_calculation() 
{
	int i;
	int size = roundUp(BLOCK_SIZE, image->getHeight()) * roundUp(BLOCK_SIZE, image->getWidth()) / BLOCK_SIZE /BLOCK_SIZE;
	*maxLuminance = maxLumArray[0];
	*avLuminance = 0;
	for(i = 0; i < size; i++)
	{
		//logFile("max = %f ", maxLumArray[i]);
		if(*maxLuminance < maxLumArray[i]) *maxLuminance = maxLumArray[i];
		*avLuminance += avLumArray[i];
	}
	float imageSize = (float)image->getWidth() * (float)image->getHeight();
	*avLuminance = exp(*avLuminance / imageSize);
}

void LuminancePixel::clearDeviceMemory()
{
	clReleaseMemObject(cl_floatImage);
	
	clReleaseMemObject(cl_avLumArray);
	delete[] avLumArray;
	
	clReleaseMemObject(cl_maxLumArray);
	delete[] maxLumArray;
}

int LuminancePixel::getNumOfDim()
{
	return 2;
}

size_t* LuminancePixel::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* LuminancePixel::getSzLocalWorkSize()
{
	return localWorkSize;
}