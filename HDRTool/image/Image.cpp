#include "Image.h"

Image::Image(void)
{
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
	return image;
}

guchar* Image::getPreviewImage()
{
	return previewImage;
}