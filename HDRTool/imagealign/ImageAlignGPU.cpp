#include "ImageAlignGPU.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"
#include "../image/ConversionRGB2BW.h"

ImageAlignGPU::ImageAlignGPU()
{
	core = new OpenCLCore(this, "imagealign/image_align.cl", "align");
}

ImageAlignGPU::~ImageAlignGPU()
{
	delete core;
}

void ImageAlignGPU::align(int num_of_image, Image<unsigned char> **imgs)
{
	images = imgs;
	num_images = num_of_image;

	width = images[0]->getWidth();
	height = images[0]->getHeight();
	double quantile = 0.5;
	noise = 4;
	shift_bits = max((int)floor(log((double)min(width, height)) / log((double)2)) - 6, 0); //calculate log for base 2 log( n ) / log( 2 ); 
	logFile("::mtb_alignment: width=%d, height=%d, shift_bits=%d\n", width, height, shift_bits);

	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	localWorkSize[2] = shift_bits;

	//round values on upper value
	logFile("%d %d \n", images[0]->getHeight(), images[0]->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, images[0]->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, images[0]->getWidth());
	globalWorkSize[2] = shift_bits;

	//these arrays contain the shifts of each image (except the 0-th) wrt the previous one
	shiftsX=new int[num_of_image - 1];
	shiftsY=new int[num_of_image - 1];

	//find the shitfs
	for (int i = 0; i < num_images - 1; i++) 
	{
		//getLum(image1, img1lum, cdf1);
		median1 = calculateLumImage(images[i], img1lum, quantile);

		//getLum(image2, img2lum, cdf2);
		median2 = calculateLumImage(images[i + 1], img2lum, quantile);
	
 		logFile("::mtb_alignment: align::medians, image 1: %d, image 2: %d\n",median1, median2);
		
		core->runComputeUnit();

		//delete img1lum; 
		//delete img2lum;

		//img1lum = NULL;
		//img2lum = NULL;
	}
}

int ImageAlignGPU::calculateLumImage(Image<unsigned char> *in, Image<unsigned char> *out, double quantile)
{
		Image<unsigned char> *imgLum=new Image<unsigned char>(1, height, width);
		long *hist = new long[256];
		double *cdf = new double[256];
		int median;
		
		//getLum(image1, img1lum, cdf1);
		ConversionRGB2BW *conv = new ConversionRGB2BW();
		conv->convertRGB2BW(in, imgLum, hist);
		double size = height * width;
		double w = 1.0 / size;
		cdf[0] = w * (double)hist[0];
		for(int i = 1; i < 256; i++) 
		{
			printf("%d ", hist[i]);
			cdf[i] = (double)cdf[i-1] + w * (double)hist[i];
		}
		delete conv;
		
		for(median = 0; median < 256; median++) 
			if( cdf[median] >= quantile ) break;

		delete []hist;
		delete []cdf;

		return median;
}

void ImageAlignGPU::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int sizeImage = sizeof(unsigned char) * height * width;
	cl_image1 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr1);
	cl_image2 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_threshold1 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_threshold2 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_mask1 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_mask2 = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_diff = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeImage, NULL, &ciErr2);
	ciErr1 |= ciErr2;

	unsigned int sizeShifts = sizeof(int) * shift_bits;
	cl_x_shift = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeShifts, NULL, &ciErr2);
    ciErr1 |= ciErr2;
	cl_y_shift = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		sizeShifts, NULL, &ciErr2);
    ciErr1 |= ciErr2;

	logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ImageAlignGPU::setInputDataToOpenCLMemory()
{
	cl_int ciErr1;				
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, 
		sizeof(cl_mem), (void*)&cl_image1);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, 
		sizeof(cl_mem), (void*)&cl_image2);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, 
		sizeof(cl_mem), (void*)&cl_threshold1);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, 
		sizeof(cl_mem), (void*)&cl_threshold2);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_mem), (void*)&cl_mask1);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 5, 
		sizeof(cl_mem), (void*)&cl_mask2);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 6, 
		sizeof(cl_mem), (void*)&cl_diff);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 7, 
		sizeof(cl_int), (void*)&median1);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 8, 
		sizeof(cl_int), (void*)&median2);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 9, 
		sizeof(cl_mem), (void*)&cl_x_shift);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 10, 
		sizeof(cl_mem), (void*)&cl_y_shift);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 11, 
		sizeof(cl_int), (void*)&shift_bits);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 12, 
		sizeof(cl_int), (void*)&noise);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 13, 
		sizeof(cl_int), (void*)&height);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 14, 
		sizeof(cl_int), (void*)&width);
    logFile("clSetKernelArg 0 - 14...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clSetKernelArg, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
	unsigned int sizeImage = sizeof(unsigned char) * height * width;
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_image1, CL_TRUE, 0, 
		sizeImage, img1lum->getImage(), 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_image2, CL_TRUE, 0, 
		sizeImage, img2lum->getImage(), 0, NULL, NULL);
    
	logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ImageAlignGPU::getDataFromOpenCLMemory()
{
	unsigned int sizeShifts = sizeof(int) * shift_bits;
	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_x_shift, CL_TRUE, 0, 
		sizeShifts, x_shift, 0, NULL, NULL);
	ciErr1 |= clEnqueueReadBuffer(core->getCqCommandQueue(), cl_y_shift, CL_TRUE, 0, 
		sizeShifts, y_shift, 0, NULL, NULL);
	logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void ImageAlignGPU::clearDeviceMemory()
{
	//TODO
	//clReleaseMemObject(cl_image);
	//clReleaseMemObject(cl_image_bw);
}

int ImageAlignGPU::getNumOfDim()
{
	return 3;
}

size_t* ImageAlignGPU::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* ImageAlignGPU::getSzLocalWorkSize()
{
	return localWorkSize;
}