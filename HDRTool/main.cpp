#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <string.h>

#include "utils/Log.h"
#include "utils/Consts.h"
#include "utils/clUtil/OpenCLUtil.h"
#include "image/fileformat/Radiance.h"
#include "VectorADD.h"
#include "image/ConversionRGB2RGBE.h"
#include "image/ConversionRGBE2RGB.h"
#include "tonemapping/LuminancePixel.h"
#include "tonemapping/ToneMappingDrago03.h"
#include "genhdr/GenerateHDRDebevec.h"
#include "genhdr/Responses.h"
#include "imagealign/ImageAlign.h"
#include "CImg.h"

using namespace cimg_library;


// OpenCL Vars
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device



size_t szParmDataBytes;			// Byte size of context information


char* cPathAndName = NULL;      // var for full paths to data, src, etc.

const char* cExecutableName = NULL;




Image<unsigned char>* loadImage(char *fileName)
{
	CImg<unsigned char> image2(fileName);
	Image<unsigned char> *img2 = new Image<unsigned char>(3, image2._height, image2._width);
	for(int y = 0; y <image2._height; y++)
	{
		for(int x = 0; x < image2._width; x++)
		{
			img2->getImage()[y * image2._width * 3 + x * 3 + 0] = image2._data[image2._width * image2._height * 0 + y * image2._width + x];
			img2->getImage()[y * image2._width * 3 + x * 3 + 1] = image2._data[image2._width * image2._height * 1 + y * image2._width + x];
			img2->getImage()[y * image2._width * 3 + x * 3 + 2] = image2._data[image2._width * image2._height * 2 + y * image2._width + x];
			//printf("%d %d %d ", img1->getImage()[y * image._width * 3 + x * 3 + 0], img1->getImage()[y * image._width * 3 + x * 3 + 1], img1->getImage()[y * image._width * 3 + x * 3 + 2]);
		}
		//printf("\n");
	}
	return img2;
}

void testRGB2RGBE()
{
	printf("size of %d \n", sizeof(unsigned char));
	printf("size of unsigned int %d \n", sizeof(unsigned int));
	printf("size of int %d \n", sizeof(int));
	printf("size of cl_int %d \n", sizeof(cl_int));

	printf("size of float %d \n", sizeof(float));
	printf("size of cl_float %d \n", sizeof(cl_float));

	printf("size of trgbe %d\n", sizeof(Trgbe));
	
	Image<float> *image = new Image<float>(3);
	image->setWidth(2400);
	image->setHeight(4500);
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 2;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = 3;
		}
	}
	

	/*for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			float *adr = &image->getHDR()[(y*image->getWidth()+x) * 3];
			printf("%f %f %f ", adr[0], adr[1], adr[2]);
		}
		printf("\n\n");
	}*/


	ConversionRGB2RGBE *conv = new ConversionRGB2RGBE();
	unsigned int *r, *g, *b, *e;
	r = new unsigned int[image->getHeight()*image->getWidth()];
	g = new unsigned int[image->getHeight()*image->getWidth()];
	b = new unsigned int[image->getHeight()*image->getWidth()];
	e = new unsigned int[image->getHeight()*image->getWidth()];
	
	printf("sad\n");
	conv->convertRGB2RGBE(image, r, g, b, e);
	
	/*for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			printf("%u %u %u %u ", r[y*image->getWidth() + x], g[y*image->getWidth() + x], b[y*image->getWidth() + x], e[y*image->getWidth() + x]);
		}
		printf("\n\n");
	}*/
}

void testRGBE2RGB()
{
	Image<float> *image = new Image<float>(3);
	image->setExposure(1.0);
	image->setWidth(1024);
	image->setHeight(512);

	image->getImage();

	unsigned int *r, *g, *b, *e;
	r = new unsigned int[image->getHeight()*image->getWidth()];
	g = new unsigned int[image->getHeight()*image->getWidth()];
	b = new unsigned int[image->getHeight()*image->getWidth()];
	e = new unsigned int[image->getHeight()*image->getWidth()];

	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			r[y * image->getWidth() + x] = 45;
			g[y * image->getWidth() + x] = 91;
			b[y * image->getWidth() + x] = 137;
			e[y * image->getWidth() + x] = 123;
		}
	}
	ConversionRGBE2RGB *conv = new ConversionRGBE2RGB();
	printf("sad\n");
	conv->convertRGBE2RGB(r, g, b, e, image);

	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			float *adr = &image->getImage()[(y*image->getWidth()+x) * 3];
			printf("%f %f %f ", adr[0], adr[1], adr[2]);
		}
		printf("\n\n");
	}

	/*for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			printf("%u %u %u %u ", r[y*image->getWidth() + x], g[y*image->getWidth() + x], b[y*image->getWidth() + x], e[y*image->getWidth() + x]);
		}
		printf("\n\n");
	}*/
}


void testLuminancePixel() 
{
	Image<float> *image = new Image<float>(3);
	image->setWidth(2400);
	image->setHeight(4500);
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = x;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = 3;
			if(x == 1234 && y == 34) image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 4567.45f;
		}
	}

	LuminancePixel *lum = new LuminancePixel();
	float avLum, maxLum;
	lum->calculate_luminance_pixel(image, &avLum, &maxLum);
	printf("avLum=%f maxLum=%f\n", avLum, maxLum);
}

void testDrago03()
{
	Image<float> *image = new Image<float>(3);
	image->setWidth(16);
	image->setHeight(16);
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 2;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = 3;
			if(x == 5 && y == 5) image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 4567.45f;
		}
	}
	ToneMappingDrago03 *tone = new ToneMappingDrago03();
	float avLum = 1;
	float maxLum = 4567.45;
	unsigned int *pic = new unsigned int[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS];
	tone->toneMapping_Drago03(image, &avLum, &maxLum, pic, 0.85);

	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			int pos = (y * image->getWidth() + x) * 3;
			printf("%d %d %d ", pic[pos + 0], pic[pos + 1], pic[pos + 2]);
		}
		printf("\n");
	}
}

void testDebevec()
{
	Image<float> *image = new Image<float>(3);
	image->setWidth(16);
	image->setHeight(16);
	unsigned int *ldr = new unsigned int[3 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS];
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			ldr[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			ldr[y * image->getWidth() * 3 + x * 3 + 1] = 1;
			ldr[y * image->getWidth() * 3 + x * 3 + 2] = 1;
			
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 0] = 2;
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 1] = 2;
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 2] = 2;

			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 0] = 3;
			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 1] = 3;
			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 2] = 3;
		}
	}
	
	GenerateHDRDebevec *genHDR = new GenerateHDRDebevec();
	float *arrayofexptime = new float[3];
	float *Ir;
	float *Ig;
	float *Ib;
	float *W;
	int M;

	int opt_bpp = 8; //info po pix	
	//either 256(LDRs) or 65536 (RAW images, 16 bit tiffs).
	M = (int) powf(2.0f,opt_bpp);
	W = new float[M];
	Responses *r = new Responses();
	r->weights_triangle(W, M);
	Ir = new float[M];
	Ig = new float[M];
	Ib = new float[M];
	r->responseLinear(Ir, M);
	r->responseLinear(Ig, M);
	r->responseLinear(Ib, M);


	genHDR->generateHDR(image, arrayofexptime, Ir, Ig, Ib, W, M, 3, ldr);
}

void testScaled2()
{
	Image<unsigned char> *image = new Image<unsigned char>(3, 32, 16);
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = 1;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 2;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = 3;
			//if(x == 5 && y == 5) image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 4567.45f;
		}
	}
	image->scaled2();
}

void testImageAligne() 
{
	Image<unsigned char> *image = new Image<unsigned char>(3);
	Image<unsigned char> *image2 = new Image<unsigned char>(3);
	image->setWidth(8);
	image->setHeight(8);

	image2->setWidth(8);
	image2->setHeight(8);
	
	unsigned int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = rand() % 256;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = rand() % 256;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = rand() % 256;

			if(y >= 2 && y<= 3 && x >= 2 && x <= 3)
			{
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = 156;
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = 223;
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = 167;
			}
		}
	}

	for(y=0; y<image2->getHeight(); y++)
	{
		for(x=0; x<image2->getWidth(); x++)
		{
			image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 0] = rand() % 256;
			image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 1] = rand() % 256;
			image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 2] = rand() % 256;

			if(y >= 4 && y<= 5 && x >= 2 && x <= 3)
			{
				image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 0] = 156;
				image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 1] = 223;
				image2->getImage()[y * image2->getWidth() * 3 + x * 3 + 2] = 167;
			}
		}
	}
	
	ImageAlign *align = new ImageAlign();
	Image<unsigned char>** image_list = new Image<unsigned char>*[2];

	bool *image_bool = new bool[2];
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		image_bool[i] = false;
		//image_list[i] = image;
	}
	image_list[0] = image;
	image_list[1] = image2;
	align->mtb_alignment(2, image_list, image_bool);
}

void testAlignJpegPic()
{
	CImg<unsigned char> image1("IMG_3582.jpg");
	Image<unsigned char> *img1 = new Image<unsigned char>(3, image1._height, image1._width);
	for(int y = 0; y <image1._height; y++)
	{
		for(int x = 0; x < image1._width; x++)
		{
			img1->getImage()[y * image1._width * 3 + x * 3 + 0] = image1._data[image1._width * image1._height * 0 + y * image1._width + x];
			img1->getImage()[y * image1._width * 3 + x * 3 + 1] = image1._data[image1._width * image1._height * 1 + y * image1._width + x];
			img1->getImage()[y * image1._width * 3 + x * 3 + 2] = image1._data[image1._width * image1._height * 2 + y * image1._width + x];
			//printf("%d %d %d ", img1->getImage()[y * image._width * 3 + x * 3 + 0], img1->getImage()[y * image._width * 3 + x * 3 + 1], img1->getImage()[y * image._width * 3 + x * 3 + 2]);
		}
		//printf("\n");
	}

	CImg<unsigned char> image2("IMG_3583.jpg");
	Image<unsigned char> *img2 = new Image<unsigned char>(3, image2._height, image2._width);
	for(int y = 0; y <image2._height; y++)
	{
		for(int x = 0; x < image2._width; x++)
		{
			img2->getImage()[y * image2._width * 3 + x * 3 + 0] = image2._data[image2._width * image2._height * 0 + y * image2._width + x];
			img2->getImage()[y * image2._width * 3 + x * 3 + 1] = image2._data[image2._width * image2._height * 1 + y * image2._width + x];
			img2->getImage()[y * image2._width * 3 + x * 3 + 2] = image2._data[image2._width * image2._height * 2 + y * image2._width + x];
			//printf("%d %d %d ", img1->getImage()[y * image._width * 3 + x * 3 + 0], img1->getImage()[y * image._width * 3 + x * 3 + 1], img1->getImage()[y * image._width * 3 + x * 3 + 2]);
		}
		//printf("\n");
	}

	ImageAlign *align = new ImageAlign();
	Image<unsigned char>** image_list = new Image<unsigned char>*[2];

	bool *image_bool = new bool[2];
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		image_bool[i] = false;
		//image_list[i] = image;
	}
	image_list[0] = img1;
	image_list[1] = img2;
	align->mtb_alignment(2, image_list, image_bool);
}

void testAligneRealImage() 
{
	FILE *file = fopen("clocks.hdr", "r");
	Radiance *radiance = new Radiance(file);
	Image<float> *image = radiance->readFile();
	printf("exposure: %f\n", image->getExposure());
	printf("height: %d\n", image->getHeight());
	printf("width: %d\n", image->getWidth());
	fclose(file);
	delete radiance;
	
	LuminancePixel *lum = new LuminancePixel();
	float avLum, maxLum;
	lum->calculate_luminance_pixel(image, &avLum, &maxLum);
	printf("avLum=%f maxLum=%f\n", avLum, maxLum);
	delete lum;

	ToneMappingDrago03 *tone = new ToneMappingDrago03();
	//float avLum = 1;
	//float maxLum = 4567.45;
	unsigned int *pic = new unsigned int[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS];
	tone->toneMapping_Drago03(image, &avLum, &maxLum, pic, 0.85);

	Image<unsigned char> *img1, *img2;
	img1 = new Image<unsigned char>(RGB_NUM_OF_CHANNELS, image->getHeight(), image->getWidth());
	img2 = new Image<unsigned char>(RGB_NUM_OF_CHANNELS, image->getHeight(), image->getWidth());
	for(int y = 0; y < image->getHeight(); y++)
	{
		for(int x = 0; x < image->getWidth(); x++)
		{
			img1->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 0] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 0];
			img1->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 1] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 1];
			img1->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 2] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 2];
			img2->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 0] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 0];
			img2->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 1] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 1];
			img2->getImage()[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 2] = pic[y * image->getWidth() * RGB_NUM_OF_CHANNELS + x * RGB_NUM_OF_CHANNELS + 2];
		}
	}

	ImageAlign *align = new ImageAlign();
	Image<unsigned char>** image_list = new Image<unsigned char>*[2];

	bool *image_bool = new bool[2];
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		image_bool[i] = false;
		//image_list[i] = image;
	}
	image_list[0] = img1;
	image_list[1] = img2;
	align->mtb_alignment(2, image_list, image_bool);
}

void testVertical()
{
	//CImg<unsigned char> image1("IMG_3582.jpg");
/*	Image<unsigned char> *img1 = loadImage("IMG_3582.jpg");
	Image<unsigned char> *img2 = loadImage("IMG_3583.jpg");
	Image<unsigned char> *img3 = loadImage("IMG_3584.jpg");;
	
	ImageAlign *align = new ImageAlign();
	Image<unsigned char>** image_list = new Image<unsigned char>*[3];

	bool *image_bool = new bool[3];
	int i = 0;
	for(i = 0; i < 3; i++)
	{
		image_bool[i] = true;
		//image_list[i] = image;
	}
	image_list[0] = img1;
	image_list[1] = img2;
	image_list[2] = img3;
	align->mtb_alignment(3, image_list, image_bool);



	Image<float> *image = new Image<float>(3, img1->getHeight(), img2->getWidth());
	unsigned int *ldr = new unsigned int[3 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS];
	int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		for(x=0; x<image->getWidth(); x++)
		{
			ldr[y * image->getWidth() * 3 + x * 3 + 0] = img1->getImage()[y * image->getWidth() * 3 + x * 3 + 0];
			ldr[y * image->getWidth() * 3 + x * 3 + 1] = img1->getImage()[y * image->getWidth() * 3 + x * 3 + 1];
			ldr[y * image->getWidth() * 3 + x * 3 + 2] = img1->getImage()[y * image->getWidth() * 3 + x * 3 + 2];
			
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 0] = img2->getImage()[y * image->getWidth() * 3 + x * 3 + 0];
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 1] = img2->getImage()[y * image->getWidth() * 3 + x * 3 + 1];
			ldr[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 2] = img2->getImage()[y * image->getWidth() * 3 + x * 3 + 2];

			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 0] = img3->getImage()[y * image->getWidth() * 3 + x * 3 + 0];
			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 1] = img3->getImage()[y * image->getWidth() * 3 + x * 3 + 1];
			ldr[2 * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS + y * image->getWidth() * 3 + x * 3 + 2] = img3->getImage()[y * image->getWidth() * 3 + x * 3 + 2];
		}
	}
	
	GenerateHDRDebevec *genHDR = new GenerateHDRDebevec();
	float *arrayofexptime = new float[3];
	arrayofexptime[0] = 1.0 / 125.0 * 100.0 / 5.6 / 5.6 / 12.07488f;
	arrayofexptime[1] = 1.0 / 500.0 * 100.0 / 5.6 / 5.6 / 12.07488f;
	arrayofexptime[2] = 1.0 / 30.0 * 100.0 / 5.6 / 5.6 / 12.07488f;
	printf("exp = %f \n", arrayofexptime[0]);
	float *Ir;
	float *Ig;
	float *Ib;
	float *W;
	int M;

	int opt_bpp = 8; //info po pix	
	//either 256(LDRs) or 65536 (RAW images, 16 bit tiffs).
	M = (int) powf(2.0f,opt_bpp);
	W = new float[M];
	Responses *r = new Responses();
	r->weights_triangle(W, M);
	Ir = new float[M];
	Ig = new float[M];
	Ib = new float[M];
	r->responseLinear(Ir, M);
	r->responseLinear(Ig, M);
	r->responseLinear(Ib, M);

	genHDR->generateHDR(image, arrayofexptime, Ir, Ig, Ib, W, M, 3, ldr);
*/

	
	
	FILE *file = fopen("proba_small.hdr", "rb");
	Radiance *radiance = new Radiance(file);
	Image<float> *image = radiance->readFile();
	printf("exposure: %f\n", image->getExposure());
	printf("height: %d\n", image->getHeight());
	printf("width: %d\n", image->getWidth());
	fclose(file);
	delete radiance;


	unsigned int x,y;
	for(y=0; y<image->getHeight(); y++)
	{
		x = 0;
		/*for(x=0; x<image->getWidth(); x++)
		{
			printf("%f %f %f ", image->getImage()[y * image->getWidth() * 3 + x * 3 + 0], 
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 1],
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 2]);
		}*/
		/*printf("%f %f %f ", image->getImage()[y * image->getWidth() * 3 + x * 3 + 0], 
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 1],
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 2]);*/
		//printf("\n");
	}
	
	
	LuminancePixel *lum = new LuminancePixel();
	float avLum, maxLum;
	lum->calculate_luminance_pixel(image, &avLum, &maxLum);
	printf("avLum=%f maxLum=%f\n", avLum, maxLum);
	delete lum;

	ToneMappingDrago03 *tone = new ToneMappingDrago03();
	//float avLum = 1;
	//float maxLum = 4567.45;
	unsigned int *pic = new unsigned int[image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS];
	tone->toneMapping_Drago03(image, &avLum, &maxLum, pic, 0.85);

	
	
	
	CImg<unsigned char> image1("IMG_3582.jpg");
	
	image1._data = new unsigned char[image->getWidth() * image->getHeight() * 3];
	image1._width = image->getWidth();
	image1._height = image->getHeight();
	for(int y = 0; y <image1._height; y++)
	{
		for(int x = 0; x < image1._width; x++)
		{
			image1._data[image->getWidth() * image->getHeight() * 0 + y * image->getWidth() + x] = pic[y * image->getWidth() * 3 + x * 3 + 0];
			image1._data[image->getWidth() * image->getHeight() * 1 + y * image->getWidth() + x] = pic[y * image->getWidth() * 3 + x * 3 + 1];
			image1._data[image->getWidth() * image->getHeight() * 2 + y * image->getWidth() + x] = pic[y * image->getWidth() * 3 + x * 3 + 2];
			//printf("%d %d %d ", img1->getImage()[y * image._width * 3 + x * 3 + 0], img1->getImage()[y * image._width * 3 + x * 3 + 1], img1->getImage()[y * image._width * 3 + x * 3 + 2]);
		}
		//printf("\n");
	}
	
	image1.save("test.bmp");

}
void testFile()
{
	/*FILE *file = fopen("clocks.hdr", "r");
	//FILE *file = fopen("proba_cuda1.hdr", "r");
	Radiance *radiance = new Radiance(file);
	Image *image = radiance->readFile();
	printf("exposure: %f\n", image->getExposure());
	printf("height: %d\n", image->getHeight());
	printf("width: %d\n", image->getWidth());
	fclose(file);
*/
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

	

	*/
	FILE *file = fopen("proba_cuda1.hdr", "r");
	Radiance *radiance = new Radiance(file);
	Image<float> *image = radiance->readFile();
	printf("exposure: %f\n", image->getExposure());
	printf("height: %d\n", image->getHeight());
	printf("width: %d\n", image->getWidth());
	fclose(file);


	/*FILE *output = NULL;
	output = fopen("test1.hdr", "w");
	if (!output)perror("fopen");
	if(output == NULL)
	{
		printf("NULL");
	}
	//Radiance *radiance = new Radiance(output);
	radiance->setFile(output);
	radiance->writeFile(image);
	fclose(output);*/

	//delete radiance;

	/*Radiance *rad = new Radiance(output);

	Trgbe_pixel * pixel = new Trgbe_pixel();
	pixel->r = 2;
	pixel->g = 3;
	pixel->b = 4;
	pixel->e = 204;
	
	float r, g, b;
	rad->rgbe2rgb(*pixel, 1.0,&r, &g, &b);
	printf("%f %f %f\n", r, g, b);
		
	rad->rgb2rgbe(r,g,b,pixel);
	unsigned int r_int = pixel->r;
	unsigned int g_int = pixel->g;
	unsigned int b_int = pixel->b;
	unsigned int e_int = pixel->e;
	printf("%d %d %d %d\n", pixel->r, pixel->g, pixel->b, pixel->e);

	rad->rgbe2rgb(*pixel, 1.0, &r, &g, &b);
	printf("%f %f %f\n", r, g, b);

	rad->rgb2rgbe(r,g,b,pixel);
	printf("%d %d %d %d\n", pixel->r, pixel->g, pixel->b, pixel->e);*/
}


int main(int argc, char **argv)
{
	//testFile();
	//testDebevec();
	//testDrago03();
	//testLuminancePixel();
	//testRGB2RGBE();
	//testRGBE2RGB();
	//testScaled2();
	//testImageAligne();
	//testAligneRealImage();
	//testAlignJpegPic();
	testVertical();

	printf("\n\nThe End\n\n");

	system("Pause");


	return 0;
}
