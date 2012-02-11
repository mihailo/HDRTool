#ifndef CONVERSION_RGB_2_BW_H
#define CONVERSION_RGB_2_BW_H


#include <CL/opencl.h>
#include "../openCLCore/OpenCLCore.h"
#include "../openCLCore/OpenCLComputeUnit.h"
#include "Image.h"


class ConversionRGB2BW:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image<unsigned char> *image;
	Image<unsigned char> *image_bw;
	long *hist;
	
	cl_mem cl_image;
	cl_mem cl_image_bw;
	cl_mem cl_hist;
	
	size_t globalWorkSize[2], localWorkSize[2];
public:
	ConversionRGB2BW();
	~ConversionRGB2BW();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void convertRGB2BW(Image<unsigned char> *image, Image<unsigned char> *image_bw, long *hist);

	size_t *getSzGlobalWorkSize();        // 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		    // 2D var for # of work items in the work group	
};


#endif