#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <string.h>

#include "utils/Log.h"
#include "utils/clUtil/OpenCLUtil.h"
#include "image/fileformat/Radiance.h"


void *srcA, *srcB, *dst;        // Host buffers for OpenCL test
void* Golden;                   // Host buffer for host golden processing cross check

// OpenCL Vars
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device
cl_program cpProgram;           // OpenCL program
cl_kernel ckKernel;             // OpenCL kernel
cl_mem cmDevSrcA;               // OpenCL device source buffer A
cl_mem cmDevSrcB;               // OpenCL device source buffer B 
cl_mem cmDevDst;                // OpenCL device destination buffer 
size_t szGlobalWorkSize = 10;        // 1D var for Total # of work items
size_t szLocalWorkSize = 10;		    // 1D var for # of work items in the work group	
size_t szParmDataBytes;			// Byte size of context information
size_t szKernelLength;			// Byte size of kernel code
cl_int ciErr1, ciErr2;			// Error code var
char* cPathAndName = NULL;      // var for full paths to data, src, etc.
char* cSourceCL = NULL;         // Buffer to hold source for compilation
const char* cExecutableName = NULL;

// demo config vars
int iNumElements = 11444777;	// Length of float arrays to process (odd # for illustration)







int main(int argc, char **argv)
{
	//Get an OpenCL platform
	/*cl_platform_id cpPlatform;
	clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get a GPU device
	cl_device_id cdDevice;
	clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);

	char cBuffer[1024];
	clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	printf("CL_DEVICE_NAME:       %s\n", cBuffer);
	clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	printf("CL_DRIVER_VERSION: %s\n\n", cBuffer);*/

	//FILE *file = fopen("clocks.hdr", "r");
	/*FILE *file = fopen("proba_cuda1.hdr", "r");
	Radiance *radiance = new Radiance(file);
	Image *image = radiance->readFile();*/

/*	Image *image = new Image();
	image->setExposure(1.1);
	image->setHeight(2);
	image->setWidth(150);

	int i,j;
	for(i=0; i<2; i++)
	{
		for(j=0; j<150; j++)
		{
			if(j<2) {
				image->getHDR()[i*150*3 + j*3 + 0] = j;
				image->getHDR()[i*150*3 + j*3 + 1] = j;
				image->getHDR()[i*150*3 + j*3 + 2] = j;
			}
			else
			{
				image->getHDR()[i*150*3 + j*3 + 0] = 1;
				image->getHDR()[i*150*3 + j*3 + 1] = 2;
				image->getHDR()[i*150*3 + j*3 + 2] = 3;
			}
			if(j>130)
			{
				image->getHDR()[i*150*3 + j*3 + 0] = j;
			image->getHDR()[i*150*3 + j*3 + 1] = j;
			image->getHDR()[i*150*3 + j*3 + 2] = j;
			}

		}
	}

	printf("\n");
//	int i,j;
	for(i=0; i<2; i++)
	{
		for(j=0; j<150; j++)
		{
			printf("%f %f %f | ", image->getHDR()[i*150*3 + j*3 + 0],
				image->getHDR()[i*150*3 + j*3 + 1],
			image->getHDR()[i*150*3 + j*3 + 2]);
		}
		printf("\n");
	}*/

	/*printf("exposure: %f\n", image->getExposure());
	printf("height: %d\n", image->getHeight());
	printf("width: %d\n", image->getWidth());
	fclose(file);*/
	/*FILE *output = NULL;
	output = fopen("test1.hdr", "w");
	if (!output)perror("fopen");
	if(output == NULL)
	{
		printf("NULL");
	}
	Radiance *radiance = new Radiance(output);
	radiance->setFile(output);
	radiance->writeFile(image);
	fclose(output);
*/
	//delete radiance;

/*	Radiance *rad = new Radiance(output);

	Trgbe_pixel * pixel = new Trgbe_pixel();
	pixel->r = 2;
	pixel->g = 3;
	pixel->b = 4;
	pixel->e = 204;
	
	float r, g, b;
	rad->rgbe2rgb(*pixel, 1.0,&r, &g, &b);
	printf("%f %f %f\n", r, g, b);
		
	rad->rgb2rgbe(r,g,b,pixel);
	printf("%d %d %d %d\n", pixel->r, pixel->g, pixel->b, pixel->e);

	rad->rgbe2rgb(*pixel, 1.0, &r, &g, &b);
	printf("%f %f %f\n", r, g, b);

	rad->rgb2rgbe(r,g,b,pixel);
	printf("%d %d %d %d\n", pixel->r, pixel->g, pixel->b, pixel->e);*/

	
	
	
	
	
	
	
	
	
	
	// Allocate and initialize host arrays 
    srcA = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    srcB = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    dst = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    Golden = (void *)malloc(sizeof(cl_float) * iNumElements);
    

    //Get an OpenCL platform
    ciErr1 = clGetPlatformIDs(1, &cpPlatform, NULL);

    shrLog("clGetPlatformID...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clGetPlatformID, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }

    //Get the devices
    ciErr1 = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
    shrLog("clGetDeviceIDs...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clGetDeviceIDs, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    //Create the context
    cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &ciErr1);
    shrLog("clCreateContext...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clCreateContext, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Create a command-queue
    cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &ciErr1);
    shrLog("clCreateCommandQueue...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Allocate the OpenCL buffer memory objects for source and result on the device GMEM
    cmDevSrcA = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);
    cmDevSrcB = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    cmDevDst = clCreateBuffer(cxGPUContext, CL_MEM_WRITE_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    shrLog("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }
    
    // Read the OpenCL kernel in from source file
    shrLog("oclLoadProgSource ()...\n"); 
    cPathAndName = "vectorADD.cl";///shrFindFilePath(cSourceFile, argv[0]);
    cSourceCL = oclLoadProgSource(cPathAndName, "", &szKernelLength);

    // Create the program
    cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &ciErr1);
    shrLog("clCreateProgramWithSource...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clCreateProgramWithSource, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Build the program with 'mad' Optimization option
    #ifdef MAC
        char* flags = "-cl-fast-relaxed-math -DMAC";
    #else
        char* flags = "-cl-fast-relaxed-math";
    #endif
    ciErr1 = clBuildProgram(cpProgram, 0, NULL, NULL, NULL, NULL);
    shrLog("clBuildProgram...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clBuildProgram, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
   //     Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Create the kernel
    ckKernel = clCreateKernel(cpProgram, "VectorAdd", &ciErr1);
    shrLog("clCreateKernel (VectorAdd)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Set the Argument values
    ciErr1 = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&cmDevSrcA);
    ciErr1 |= clSetKernelArg(ckKernel, 1, sizeof(cl_mem), (void*)&cmDevSrcB);
    ciErr1 |= clSetKernelArg(ckKernel, 2, sizeof(cl_mem), (void*)&cmDevDst);
    ciErr1 |= clSetKernelArg(ckKernel, 3, sizeof(cl_int), (void*)&iNumElements);
    shrLog("clSetKernelArg 0 - 3...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clSetKernelArg, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
    ciErr1 = clEnqueueWriteBuffer(cqCommandQueue, cmDevSrcA, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcA, 0, NULL, NULL);
    ciErr1 |= clEnqueueWriteBuffer(cqCommandQueue, cmDevSrcB, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);
    shrLog("clEnqueueWriteBuffer (SrcA and SrcB)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Launch kernel
    ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &szGlobalWorkSize, &szLocalWorkSize, 0, NULL, NULL);
    shrLog("clEnqueueNDRangeKernel (VectorAdd)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
     //   Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Synchronous/blocking read of results, and check accumulated errors
    ciErr1 = clEnqueueReadBuffer(cqCommandQueue, cmDevDst, CL_TRUE, 0, sizeof(cl_float) * szGlobalWorkSize, dst, 0, NULL, NULL);
    shrLog("clEnqueueReadBuffer (Dst)...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        shrLog("Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
    //    Cleanup(argc, argv, EXIT_FAILURE);
    }
    //--------------------------------------------------------

    // Compute and compare results for golden-host and report errors and pass/fail
    shrLog("Comparing against Host/C++ computation...\n\n"); 
    //VectorAddHost ((const float*)srcA, (const float*)srcB, (float*)Golden, iNumElements);
    //shrBOOL bMatch = shrComparefet((const float*)Golden, (const float*)dst, (unsigned int)iNumElements, 0.0f, 0);

    // Cleanup and leave
    //Cleanup (argc, argv, (bMatch == shrTRUE) ? EXIT_SUCCESS : EXIT_FAILURE);

	
	
	printf("\n\nThe End\n\n");

	system("Pause");


	return 0;
}
