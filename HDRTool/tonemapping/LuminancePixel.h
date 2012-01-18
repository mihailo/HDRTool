#ifndef LUMINANCE_PIXEL_H
#define LUMINANCE_PIXEL_H


#include "../openCLCore/OpenCLComputeUnit.h"
#include "../openCLCore/OpenCLCore.h"
#include "../image/Image.h"


class LuminancePixel:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image<float> *image;
	float *avLuminance,	*maxLuminance;
	float *avLumArray, *maxLumArray;
	
	cl_mem cl_floatImage;
	cl_mem cl_avLumArray, cl_maxLumArray;

	size_t globalWorkSize[2], localWorkSize[2];

	void final_calculation();
public:
	LuminancePixel();
	~LuminancePixel();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void calculate_luminance_pixel(Image<float> *img, float *avLum, float *maxLum);

	size_t *getSzGlobalWorkSize();		// 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		// 2D var for # of work items in the work group	
};

#endif
