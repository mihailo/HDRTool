#ifndef CONVERSION_RGBE_2_RGB_H
#define CONVERSION_RGBE_2_RGB_H


#include <CL/opencl.h>
#include "../openCLCore/OpenCLCore.h"
#include "../openCLCore/OpenCLComputeUnit.h"
#include "Image.h"


class ConversionRGBE2RGB:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image *image;
	unsigned int *channelR;
	unsigned int *channelG;
	unsigned int *channelB;
	unsigned int *channelE;

	cl_mem cl_floatImage;
	cl_mem cl_intChannelR;
	cl_mem cl_intChannelG;
	cl_mem cl_intChannelB;
	cl_mem cl_intChannelE;
	
	size_t globalWorkSize[2], localWorkSize[2];
public:
	ConversionRGBE2RGB();
	~ConversionRGBE2RGB();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void convertRGBE2RGB(unsigned int *channelR, unsigned int *channelG, unsigned int *channelB, unsigned int *channelE, Image *image);

	size_t *getSzGlobalWorkSize();        // 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		  // 2D var for # of work items in the work group	
};


#endif