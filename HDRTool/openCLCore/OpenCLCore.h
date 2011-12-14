

#ifndef OPEN_CL_CORE_H
#define OPEN_CL_CORE_H

#include <CL/opencl.h>


#include "OpenClComputeUnit.h"


class OpenCLCore 
{
private:
	char *sourceFile;
	OpenClComputeUnit *compute;

	cl_platform_id cpPlatform;      // OpenCL platform
	cl_device_id cdDevice;          // OpenCL device
	cl_context cxGPUContext;        // OpenCL context
	cl_command_queue cqCommandQueue;// OpenCL command que
	char* cSourceCL;         // Buffer to hold source for compilation
	size_t szKernelLength;			// Byte size of kernel code
	cl_program cpProgram;           // OpenCL program
	cl_kernel ckKernel;             // OpenCL kernel

	void setupOpenClPlatform();
	void createKernel();
	void runKernel();
public:
	OpenCLCore(OpenClComputeUnit *computeUnit, char *clSourceFile);
	~OpenCLCore();

	void runComputeUnit();
	cl_context getGPUContext();
	cl_kernel getOpenCLKernel();
	cl_command_queue getCqCommandQueue();
};

#endif
