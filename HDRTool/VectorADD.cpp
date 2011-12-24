#include "VectorADD.h"

#include <stdlib.h>

#include "utils/Log.h"


void VectorADD::start()
{
	//int szGlobalWorkSize[2];        // 1D var for Total # of work items
	//int intszLocalWorkSize[2];

	szLocalWorkSize[0] = 2;
	szLocalWorkSize[1] = 1;
	
	szGlobalWorkSize[0] = 2;
	szGlobalWorkSize[1] = 5;
	
	core = new OpenCLCore(this, "vectorADD.cl", "VectorAdd");
	core->runComputeUnit();
}

void VectorADD::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;			// Error code var
	
	// Allocate and initialize host arrays 
    srcA = (float *)malloc(sizeof(cl_float) * 10);
    srcB = (float *)malloc(sizeof(cl_float) * 10);
    dst = (float *)malloc(sizeof(cl_float) * 10);
    Golden = (float *)malloc(sizeof(cl_float) * iNumElements);
    
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		(srcA[i]) = i;
		(srcB[i]) = i;
		(dst[i]) = i;
	}

   

    // Allocate the OpenCL buffer memory objects for source and result on the device GMEM
	cmDevSrcA = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_ONLY, sizeof(cl_float) * 10, NULL, &ciErr1);
    cmDevSrcB = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_ONLY, sizeof(cl_float) * 10, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    cmDevDst = clCreateBuffer(core->getGPUContext(), CL_MEM_WRITE_ONLY, sizeof(cl_float) * 10, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        logFile("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }
}
void VectorADD::setInputDataToOpenCLMemory()
{
	cl_int ciErr1;			// Error code var	
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, sizeof(cl_mem), (void*)&cmDevSrcA);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, sizeof(cl_mem), (void*)&cmDevSrcB);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, sizeof(cl_mem), (void*)&cmDevDst);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, sizeof(cl_int), (void*)&iNumElements);
    logFile("clSetKernelArg 0 - 3...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        logFile("Error in clSetKernelArg, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cmDevSrcA, CL_FALSE, 0, sizeof(cl_float) * 10, srcA, 0, NULL, NULL);
    ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cmDevSrcB, CL_FALSE, 0, sizeof(cl_float) * 10, srcB, 0, NULL, NULL);
    logFile("clEnqueueWriteBuffer (SrcA and SrcB)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        logFile("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

}

void VectorADD::getDataFromOpenCLMemory()
{

	cl_int ciErr1;			// Error code var
	// Synchronous/blocking read of results, and check accumulated errors
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cmDevDst, CL_TRUE, 0, sizeof(cl_float) * 10, dst, 0, NULL, NULL);
    logFile("clEnqueueReadBuffer (Dst)...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        logFile("Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }
    //--------------------------------------------------------

    // Compute and compare results for golden-host and report errors and pass/fail
    logFile("Comparing against Host/C++ computation...\n\n"); 
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		logFile("%f ", dst[i]);
	}
	logFile("\n");
    //VectorAddHost ((const float*)srcA, (const float*)srcB, (float*)Golden, iNumElements);
    //shrBOOL bMatch = shrComparefet((const float*)Golden, (const float*)dst, (unsigned int)iNumElements, 0.0f, 0);

    // Cleanup and leave
    //Cleanup (argc, argv, (bMatch == shrTRUE) ? EXIT_SUCCESS : EXIT_FAILURE);

}

size_t* VectorADD::getSzGlobalWorkSize()
{
	return szGlobalWorkSize;
}
size_t* VectorADD::getSzLocalWorkSize()
{
	return szLocalWorkSize;
}