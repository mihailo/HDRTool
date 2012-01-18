#include "ConversionRGBE2RGB.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"

ConversionRGBE2RGB::ConversionRGBE2RGB()
{
	core = new OpenCLCore(this, "image/rgbe2rgb.cl", "rgbe2rgb");
}

ConversionRGBE2RGB::~ConversionRGBE2RGB()
{
	delete core;
}

void ConversionRGBE2RGB::convertRGBE2RGB( 
					 unsigned int *r, 
					 unsigned int *g, 
					 unsigned int *b, 
					 unsigned int *e,
					 Image<float> *img)
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

void ConversionRGBE2RGB::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int sizeRGBE = sizeof(cl_uint) * image->getHeight() * image->getWidth();
	cl_intChannelR = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr1);
	cl_intChannelG = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_intChannelB = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_intChannelE = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeRGBE, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	cl_floatImage = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	
	logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGBE2RGB::setInputDataToOpenCLMemory()
{
	int height = image->getHeight();
	int width = image->getWidth();
	float exposure = image->getExposure();
	
	cl_int ciErr1, ciErr2;				
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
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 7, 
		sizeof(cl_float), (void*)&exposure);
    logFile("clSetKernelArg 0 - 7...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clSetKernelArg, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
	unsigned int size = sizeof(cl_uint) * image->getHeight() * image->getWidth();
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_intChannelR, CL_TRUE, 0, 
		size, channelR, 0, NULL, NULL);
    ciErr2 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_intChannelG, CL_TRUE, 0, 
		size, channelG, 0, NULL, NULL);
    ciErr1 |= ciErr2;
	ciErr2 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_intChannelB, CL_TRUE, 0, 
		size, channelB, 0, NULL, NULL);
    ciErr1 |= ciErr2;
	ciErr2 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_intChannelE, CL_TRUE, 0, 
		size, channelE, 0, NULL, NULL);
	ciErr1 |= ciErr2;
	
	logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGBE2RGB::getDataFromOpenCLMemory()
{
	unsigned int size = image->getHeight() * image->getWidth() * sizeof(cl_float) * RGB_NUM_OF_CHANNELS;
	cl_int ciErr1;			// Error code var
	
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_floatImage, CL_TRUE, 0, 
		size, image->getImage(), 0, NULL, NULL);
    logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGBE2RGB::clearDeviceMemory()
{
	clReleaseMemObject(cl_floatImage);
	clReleaseMemObject(cl_intChannelR);
	clReleaseMemObject(cl_intChannelG);
	clReleaseMemObject(cl_intChannelB);
	clReleaseMemObject(cl_intChannelE);
}

size_t* ConversionRGBE2RGB::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* ConversionRGBE2RGB::getSzLocalWorkSize()
{
	return localWorkSize;
}