#include "OpenCLCore.h"


#include "../utils/Log.h"
#include "../utils/clUtil/OpenCLUtil.h"
#include "../utils/timer/hr_time.h"


OpenCLCore::OpenCLCore(OpenClComputeUnit *computeUnit, char *clSourceFile, char *kName)
{
	compute = computeUnit;
	sourceFile = clSourceFile;
	kernelName = kName;
	setupOpenClPlatform();
}

void OpenCLCore::runComputeUnit()
{
	compute->allocateOpenCLMemory();
	createKernel();
	compute->setInputDataToOpenCLMemory();
	runKernel();
	compute->getDataFromOpenCLMemory();
	compute->clearDeviceMemory();
	releaseKernel();
}

OpenCLCore::~OpenCLCore()
{
}

void OpenCLCore::setupOpenClPlatform()
{
	cl_int clError;
	
	//Get an OpenCL platform
    clError = clGetPlatformIDs(1, &cpPlatform, NULL);

    logFile("clGetPlatformID...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clGetPlatformID, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    }

    //Get the devices
    clError = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
    logFile("clGetDeviceIDs...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clGetDeviceIDs, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    }
	char cBuffer[1024];
	
	clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	logFile("CL_DEVICE_NAME:       %s\n", cBuffer);
	clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	logFile("CL_DRIVER_VERSION: %s\n", cBuffer);
	clGetDeviceInfo(cdDevice, CL_PLATFORM_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	logFile("CL_PLATFORM_VERSION: %s\n\n", cBuffer);
	
    //Create the context
    cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &clError);
    logFile("clCreateContext...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clCreateContext, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    }

    // Create a command-queue
    cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &clError);
    logFile("clCreateCommandQueue...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    }
}

void OpenCLCore::createKernel()
{
	cl_int clError;
	// Read the OpenCL kernel in from source file
    logFile("oclLoadProgSource ()...\n"); 
	cSourceCL = oclLoadProgSource(sourceFile, "", &szKernelLength);

    // Create the program
    cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &clError);
    logFile("clCreateProgramWithSource...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clCreateProgramWithSource, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Build the program with 'mad' Optimization option
    /*#ifdef MAC
        char* flags = "-cl-fast-relaxed-math -DMAC";
    #else
        char* flags = "-cl-fast-relaxed-math";
    #endif*/
    clError = clBuildProgram(cpProgram, 0, NULL, NULL, NULL, NULL);
    logFile("clBuildProgram...\n"); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clBuildProgram, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
   //     Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Create the kernel
    ckKernel = clCreateKernel(cpProgram, kernelName, &clError);
    logFile("clCreateKernel (%s)...\n", kernelName); 
    if (clError != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateKernel, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }
}

void OpenCLCore::runKernel()
{
	// Launch kernel
    cl_int clError;
	clFinish(cqCommandQueue);
	CStopWatch timer;
	timer.startTimer();
	clError = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, compute->getNumOfDim(), NULL, compute->getSzGlobalWorkSize(), compute->getSzLocalWorkSize(), 0, NULL, NULL);
	clFinish(cqCommandQueue);
	timer.stopTimer();
	timer.getElapsedTime();
	logFile(" ,calc_time, , ,%f, \n", timer.getElapsedTime());
	logFile("clEnqueueNDRangeKernel (%s)...\n", kernelName); 
    if (clError != CL_SUCCESS)
    {
        logFile("%d :Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", clError, __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }
}

void OpenCLCore::releaseKernel()
{
	clReleaseKernel(ckKernel);
	clReleaseCommandQueue(cqCommandQueue);
	clReleaseContext(cxGPUContext);
}

cl_context OpenCLCore::getGPUContext()
{
	return cxGPUContext;
}

cl_kernel OpenCLCore::getOpenCLKernel()
{
	return ckKernel;
}

cl_command_queue OpenCLCore::getCqCommandQueue()
{
	return cqCommandQueue;
}