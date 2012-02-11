#ifndef IMAGE_ALIGN_GPU_H
#define IMAGE_ALIGN_GPU_H


#include <CL/opencl.h>
#include "../openCLCore/OpenCLCore.h"
#include "../openCLCore/OpenCLComputeUnit.h"
#include "../image/Image.h"


class ImageAlignGPU:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image<unsigned char> **images;
	int num_images;

	int width, height;
	int shift_bits, noise;
	Image<unsigned char> *img1lum;
	Image<unsigned char> *img2lum;
	int median1,median2;
	int *x_shift, *y_shift;
	
	cl_mem cl_image1;
	cl_mem cl_image2;
	cl_mem cl_threshold1, cl_threshold2;
	cl_mem cl_mask1, cl_mask2;
	cl_mem cl_diff;
	cl_mem cl_x_shift, cl_y_shift;

	int * shiftsX, *shiftsY;
	
	size_t globalWorkSize[2], localWorkSize[2];

	int calculateLumImage(Image<unsigned char> *in, Image<unsigned char> *out, double quantile);
public:
	ImageAlignGPU();
	~ImageAlignGPU();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void align(int num_of_image, Image<unsigned char> **images);

	size_t *getSzGlobalWorkSize();        // 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		    // 2D var for # of work items in the work group	
};


#endif