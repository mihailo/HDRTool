#include "ConversionRGB2RGBE.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"

ConversionRGB2RGBE::ConversionRGB2RGBE()
{
	core = new OpenCLCore(this, "image/rgb2rgbe.cl", "rgb2rgbe");
}

ConversionRGB2RGBE::~ConversionRGB2RGBE()
{
	delete core;
	delete[] globalWorkSize;
	delete[] localWorkSize;
}

void ConversionRGB2RGBE::convertRGB2RGBE(Image *img, 
					 unsigned int *r, 
					 unsigned int *g, 
					 unsigned int *b, 
					 unsigned int *e)
{
	image = img;

	channelR = r;
	channelG = g;
	channelB = b;
	channelE = e;

	
	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	
	//round values on upper value
	logFile("%d %d \n", image->getHeight(), image->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, image->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, image->getWidth());

	core->runComputeUnit();
}

void ConversionRGB2RGBE::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * 3;
	cl_floatImage = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr1);

	unsigned int sizeRGBE = sizeof(cl_uint) * image->getHeight() * image->getWidth();
	cl_intChannelR = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_intChannelG = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_intChannelB = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_intChannelE = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2RGBE::setInputDataToOpenCLMemory()
{
	int height = image->getHeight();
	int width = image->getWidth();
	
	cl_int ciErr1;				
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, 
		sizeof(cl_mem), (void*)&cl_floatImage);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, 
		sizeof(cl_mem), (void*)&cl_intChannelR);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, 
		sizeof(cl_mem), (void*)&cl_intChannelG);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, 
		sizeof(cl_mem), (void*)&cl_intChannelB);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_mem), (void*)&cl_intChannelE);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 5, 
		sizeof(cl_int), (void*)&height);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 6, 
		sizeof(cl_int), (void*)&width);
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
		size, image->getHDR(), 0, NULL, NULL);
    logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2RGBE::getDataFromOpenCLMemory()
{
	unsigned int size = image->getHeight() * image->getWidth() * sizeof(cl_uint);
	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_intChannelR, CL_TRUE, 0, 
		size, channelR, 0, NULL, NULL);
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_intChannelG, CL_TRUE, 0, 
		size, channelG, 0, NULL, NULL);
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_intChannelB, CL_TRUE, 0, 
		size, channelB, 0, NULL, NULL);
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_intChannelE, CL_TRUE, 0, 
		size, channelE, 0, NULL, NULL);
    logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2RGBE::clearDeviceMemory()
{
	clReleaseMemObject(cl_floatImage);
	clReleaseMemObject(cl_intChannelR);
	clReleaseMemObject(cl_intChannelG);
	clReleaseMemObject(cl_intChannelB);
	clReleaseMemObject(cl_intChannelE);
}

size_t* ConversionRGB2RGBE::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* ConversionRGB2RGBE::getSzLocalWorkSize()
{
	return localWorkSize;
}