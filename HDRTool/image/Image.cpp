#include "Image.h"

#include <stdio.h>

#include "../utils/Consts.h"
Image::Image(void)
{
	image = NULL;
	previewImage = NULL;
}

Image::Image(unsigned int h, unsigned int w)
{
	image = NULL;
	previewImage = NULL;
	height = h;
	width = w;
}

Image::~Image(void)
{
}

unsigned int Image::getHeight()
{
	return height;
}

void Image::setHeight(unsigned int newHeight)
{
	height = newHeight;
}
	
unsigned int Image::getWidth()
{
	return width;
}

void Image::setWidth(unsigned int newWidth)
{
	width = newWidth;
}

float Image::getExposure()
{
	return exposure;
}

void Image::setExposure(float newExposure)
{
	exposure = newExposure;
}

float* Image::getHDR()
{
	if(image == NULL)
	{
		image = new float[height * width * RGB_NUM_OF_CHANNELS];
	}
	return image;
}

void Image::setHDR(float *newHDR) 
{
	image = newHDR;
}

unsigned int* Image::getPreviewImage()
{
	if(previewImage == NULL)
	{
		previewImage = new unsigned int[height * width * RGB_NUM_OF_CHANNELS];
	}
	return previewImage;
}

void Image::setPreviewImage(unsigned int *newPreviewImage)
{
	previewImage = newPreviewImage;
}

void Image::fill(float r, float g, float b)
{
	float * hdr_img = getHDR();
	unsigned int x,y;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			hdr_img[(y * width + x) * RGB_NUM_OF_CHANNELS + 0] = r;
			hdr_img[(y * width + x) * RGB_NUM_OF_CHANNELS + 1] = g;
			hdr_img[(y * width + x) * RGB_NUM_OF_CHANNELS + 2] = b;
		}
	}
}