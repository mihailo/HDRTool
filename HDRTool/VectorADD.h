#ifndef VECTOR_ADD_H
#define VECTOR_ADD_H

#include <CL/opencl.h>
#include "openCLCore/OpenCLCore.h"
#include "openCLCore/OpenCLComputeUnit.h"


class VectorADD:public OpenClComputeUnit
{
private:
	OpenCLCore *core;

	void *srcA, *srcB, *dst;        // Host buffers for OpenCL test
	void* Golden;                   // Host buffer for host golden processing cross check

	size_t szGlobalWorkSize;        // 1D var for Total # of work items
	size_t szLocalWorkSize;		    // 1D var for # of work items in the work group	

	// demo config vars
	static const int iNumElements = 11444777;	// Length of float arrays to process (odd # for illustration)

	cl_mem cmDevSrcA;               // OpenCL device source buffer A
	cl_mem cmDevSrcB;               // OpenCL device source buffer B 
	cl_mem cmDevDst;                // OpenCL device destination buffer 

public:
	void start();
	void allocateOpenCLMemory();
	void setInputDataToOpenCLMemory();
	void getDataFromOpenCLMemory();

	size_t *getSzGlobalWorkSize();        // 1D var for Total # of work items
	size_t *getSzLocalWorkSize();		    // 1D var for # of work items in the work group	
};


#endif