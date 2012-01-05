#include "Image.h"

#include <stdio.h>

#include "../utils/Consts.h"
Image::Image(void)
{
	image = NULL;
	previewImage = NULL;
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