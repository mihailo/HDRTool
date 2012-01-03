#ifndef TONE_MAPPING_DRAGO_03_H
#define TONE_MAPPING_DRAGO_03_H

#include "../openCLCore/OpenCLComputeUnit.h"
#include "../openCLCore/OpenCLCore.h"
#include "../image/Image.h"


class ToneMappingDrago03:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image *image;
	unsigned int *pic;
	float *avLuminance, *maxLuminance;
	
	cl_mem cl_floatImage;
	cl_mem cl_pic;

	size_t globalWorkSize[2], localWorkSize[2];
public:
	ToneMappingDrago03();
	~ToneMappingDrago03();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void toneMapping_Drago03(Image *image, float *avLum, float maxLum, unsigned int *picture);

	size_t *getSzGlobalWorkSize();		// 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		// 2D var for # of work items in the work group	
};

#endif
