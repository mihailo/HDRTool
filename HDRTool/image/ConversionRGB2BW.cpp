#include "ConversionRGB2BW.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"
#include "../utils/timer/hr_time.h"

ConversionRGB2BW::ConversionRGB2BW()
{
	core = new OpenCLCore(this, "image/rgb2bw.cl", "rgb2bw");
}

ConversionRGB2BW::~ConversionRGB2BW()
{
	delete core;
}

void ConversionRGB2BW::convertRGB2BW(Image<unsigned char> *img, Image<unsigned char> *img_bw, long *hi)
{
	image = img;
	image_bw = img_bw;
	hist = hi;

	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	
	//round values on upper value
	logFile("%d %d \n", image->getHeight(), image->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, image->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, image->getWidth());

	core->runComputeUnit();
}

void ConversionRGB2BW::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int size = sizeof(unsigned char) * image->getHeight() * image->getWidth() * 3;
	cl_image = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr1);

	unsigned int sizeBW = sizeof(unsigned char) * image->getHeight() * image->getWidth();
	cl_image_bw = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeBW, NULL, &ciErr2);
    ciErr1 |= ciErr2;

	unsigned int sizeHist = sizeof(long) * 256;
	cl_hist = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeHist, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	
	logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2BW::setInputDataToOpenCLMemory()
{
	int height = image->getHeight();
	int width = image->getWidth();
	
	cl_int ciErr1;				
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, 
		sizeof(cl_mem), (void*)&cl_image);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, 
		sizeof(cl_mem), (void*)&cl_image_bw);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, 
		sizeof(cl_mem), (void*)&cl_hist);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, 
		sizeof(cl_int), (void*)&height);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_int), (void*)&width);
    logFile("clSetKernelArg 0 - 3...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clSetKernelArg, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    clFinish(core->getCqCommandQueue());
	CStopWatch timer;
	timer.startTimer();
	
	// Asynchronous write of data to GPU device
	unsigned int size = sizeof(unsigned char) * image->getHeight() * image->getWidth() * 3;
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_image, CL_TRUE, 0, 
		size, image->getImage(), 0, NULL, NULL);
    
	clFinish(core->getCqCommandQueue());
	timer.stopTimer();
	logFile("gpuRGB2BW,data_in,%d,%d,%f, \n", height, width, timer.getElapsedTime());
	
	logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2BW::getDataFromOpenCLMemory()
{
	
	clFinish(core->getCqCommandQueue());
	CStopWatch timer;
	timer.startTimer();

	unsigned int size = image->getHeight() * image->getWidth() * sizeof(unsigned char);
	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_image_bw, CL_TRUE, 0, 
		size, image_bw->getImage(), 0, NULL, NULL);
	unsigned int sizeHist = sizeof(long) * 256;
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_hist, CL_TRUE, 0, 
		sizeHist, hist, 0, NULL, NULL);

	clFinish(core->getCqCommandQueue());
	timer.stopTimer();
	logFile("gpuRGB2BW,data_out,%d,%d,%f, \n", image->getHeight(), image->getWidth(), timer.getElapsedTime());


	logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ConversionRGB2BW::clearDeviceMemory()
{
	clReleaseMemObject(cl_image);
	clReleaseMemObject(cl_image_bw);
}

int ConversionRGB2BW::getNumOfDim()
{
	return 2;
}

size_t* ConversionRGB2BW::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* ConversionRGB2BW::getSzLocalWorkSize()
{
	return localWorkSize;
}