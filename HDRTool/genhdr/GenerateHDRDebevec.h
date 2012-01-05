#ifndef GENERATE_HDR_DEBEVEC_H
#define GENERATE_HDR_DEBEVEC_H

#include "../openCLCore/OpenCLComputeUnit.h"
#include "../openCLCore/OpenCLCore.h"
#include "../image/Image.h"


class GenerateHDRDebevec:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	Image *image;
	int *i_lower, *i_upper;
	float * array_of_exp_time;
	float *ir, *ig, *ib;
	float *w;
	int m, max_m, min_m;
	int num_ldr;
	unsigned int **ldr;
	
	cl_mem cl_ldr_img;
	cl_mem cl_array_of_exp_time;
	cl_mem cl_i_lower, cl_i_upper;
	cl_mem cl_w;
	cl_mem cl_ir, cl_ig, cl_ib;
	cl_mem cl_hdr, cl_hdrpic;

	size_t globalWorkSize[2], localWorkSize[2];
public:
	GenerateHDRDebevec();
	~GenerateHDRDebevec();
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();
	void clearDeviceMemory();

	void generateHDR(Image *img, float *arrayofexptime, float *Ir, float *Ig, float* Ib, float *W, int M, int numimg, unsigned int **ldr_img);

	size_t *getSzGlobalWorkSize();		// 2D var for Total # of work items
	size_t *getSzLocalWorkSize();		// 2D var for # of work items in the work group	
};

#endif
