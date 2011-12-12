#include "Image.h"

#include <stdio.h>

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
		image = new float[height * width * 3];
	}
	return image;
}

void Image::setHDR(float *newHDR) 
{
	image = newHDR;
}

guchar* Image::getPreviewImage()
{
	if(previewImage == NULL)
	{
		previewImage = new guchar[height * width * 3];
	}
	return previewImage;
}

void Image::setPreviewImage(guchar *newPreviewImage)
{
	previewImage = newPreviewImage;
}