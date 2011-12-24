#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <string.h>

#include "utils/Log.h"
#include "utils/clUtil/OpenCLUtil.h"
#include "image/fileformat/Radiance.h"
#include "VectorADD.h"
#include "image/ConversionRGB2RGBE.h"




// OpenCL Vars
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device



size_t szParmDataBytes;			// Byte size of context information


char* cPathAndName = NULL;      // var for full paths to data, src, etc.

const char* cExecutableName = NULL;









int main(int argc, char **argv)
{
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

	/*VectorADD vector;
	vector.start();*/

	printf("size of %d \n", sizeof(unsigned char));
	printf("size of unsigned int %d \n", sizeof(unsigned int));
	printf("size of int %d \n", sizeof(int));
	printf("size of cl_int %d \n", sizeof(cl_int));

	printf("size of float %d \n", sizeof(float));
	printf("size of cl_float %d \n", sizeof(cl_float));

	printf("size of trgbe %d\n", sizeof(Trgbe));
	
	Image *image = new Image();
	image->setWidth(16);
	image->setHeight(32);
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getHDR()[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			image->getHDR()[y * image->getWidth() * 3 + x * 3 + 1] = 2;
			image->getHDR()[y * image->getWidth() * 3 + x * 3 + 2] = 3;
		}
	}

	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			float *adr = &image->getHDR()[(y*image->getWidth()+x) * 3];
			printf("%f %f %f ", adr[0], adr[1], adr[2]);
		}
		printf("\n\n");
	}


	ConversionRGB2RGBE *conv = new ConversionRGB2RGBE();
	unsigned int *r, *g, *b, *e;
	r = new unsigned int[image->getHeight()*image->getWidth()];
	g = new unsigned int[image->getHeight()*image->getWidth()];
	b = new unsigned int[image->getHeight()*image->getWidth()];
	e = new unsigned int[image->getHeight()*image->getWidth()];
	conv->convertRGB2RGBE(image, r, g, b, e);
	
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			printf("%u %u %u %u ", r[y*image->getWidth() + x], g[y*image->getWidth() + x], b[y*image->getWidth() + x], e[y*image->getWidth() + x]);
		}
		printf("\n\n");
	}
	
	printf("\n\nThe End\n\n");

	system("Pause");


	return 0;
}
