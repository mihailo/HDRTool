#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "image/fileformat/Radiance.h"

int main(int argc, char **argv)
{
	//Get an OpenCL platform
	cl_platform_id cpPlatform;
	clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get a GPU device
	cl_device_id cdDevice;
	clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);

	char cBuffer[1024];
	clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	printf("CL_DEVICE_NAME:       %s\n", cBuffer);
	clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	printf("CL_DRIVER_VERSION: %s\n\n", cBuffer);

	//FILE *file = fopen("clocks.hdr", "r");
	FILE *file = fopen("memorial.hdr", "r");
	Radiance *radiance = new Radiance(file);
	radiance->readFile();
	printf("\n\nThe End\n\n");

	system("Pause");

	return 0;
}
