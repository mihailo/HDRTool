#include "ToneMappingDrago03.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"

ToneMappingDrago03::ToneMappingDrago03()
{
	core = new OpenCLCore(this, "tonemapping/tone_mapping_drago_03.cl", "tone_mapping_drago03");
}

ToneMappingDrago03::~ToneMappingDrago03()
{
	delete core;
}

void ToneMappingDrago03::toneMapping_Drago03(Image<float> *img, float *avLum, float *maxLum, unsigned int *pic, float bias)
{
	image = img;
	picture = pic;

	avLuminance = avLum;
	maxLuminance = maxLum;

	normMaxLum = *maxLum / *avLum; // normalize maximum luminance by average luminance
	const float LOG05 = -0.693147f; // log(0.5)

	divider = log10(normMaxLum + 1.0f);
	biasP = log(bias)/LOG05;
	logFile("divider = %f biasP = %f \n", divider, biasP);

	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	
	//round values on upper value
	logFile("%d %d \n", image->getHeight(), image->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, image->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, image->getWidth());

	core->runComputeUnit();
}

void ToneMappingDrago03::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	cl_floatImage = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr1);

	unsigned int size_uint = sizeof(cl_uint) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	cl_picture = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_uint, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ToneMappingDrago03::setInputDataToOpenCLMemory()
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
		sizeof(cl_mem), (void*)&cl_picture);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_float), (void*)avLuminance);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 5, 
		sizeof(cl_float), (void*)&normMaxLum);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 6, 
		sizeof(cl_float), (void*)&biasP);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 7, 
		sizeof(cl_float), (void*)&divider);
   
    logFile("clSetKernelArg 0 - 7...\n\n"); 
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

void ToneMappingDrago03::getDataFromOpenCLMemory()
{
	unsigned int size = image->getHeight() * image->getWidth() * sizeof(cl_uint) * RGB_NUM_OF_CHANNELS;
	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_picture, CL_TRUE, 0, 
		size, picture, 0, NULL, NULL);
    logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ToneMappingDrago03::clearDeviceMemory()
{
	clReleaseMemObject(cl_floatImage);
	clReleaseMemObject(cl_picture);
}

size_t* ToneMappingDrago03::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* ToneMappingDrago03::getSzLocalWorkSize()
{
	return localWorkSize;
}