#ifndef TONE_MAPPING_DRAGO_03_H
#define TONE_MAPPING_DRAGO_03_H

#include "../openCLCore/OpenCLComputeUnit.h"
#include "../openCLCore/OpenCLCore.h"
#include "../image/Image.h"


class ToneMappingDrago03:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image<float> *image;
	unsigned int *picture;
	float *avLuminance, *maxLuminance;
	float normMaxLum;
	float divider;
	float biasP;
	
	cl_mem cl_floatImage;
	cl_mem cl_picture;

	size_t globalWorkSize[2], localWorkSize[2];
public:
	ToneMappingDrago03();
	~ToneMappingDrago03();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void toneMapping_Drago03(Image<float> *image, float *avLum, float *maxLum, unsigned int *picture, float bias);

	int getNumOfDim();
	size_t *getSzGlobalWorkSize();		// 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		// 2D var for # of work items in the work group	
};

#endif
