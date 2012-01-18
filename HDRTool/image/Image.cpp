#include "Image.h"

#include <stdio.h>
#include <stdlib.h>

#include "../utils/Consts.h"

template <class Type>
Image<Type>::Image(unsigned int num_channels)
{
	num_of_channels = num_channels;
	image = NULL;
	previewImage = NULL;
}

template <class Type>
Image<Type>::Image(unsigned int num_channels, unsigned int h, unsigned int w)
{
	num_of_channels = num_channels;
	image = NULL;
	previewImage = NULL;
	height = h;
	width = w;
}

template <class Type>
Image<Type>::~Image(void)
{
}

template <class Type>
unsigned int Image<Type>::getHeight()
{
	return height;
}

template <class Type>
void Image<Type>::setHeight(unsigned int newHeight)
{
	height = newHeight;
}
	
template <class Type>
unsigned int Image<Type>::getWidth()
{
	return width;
}

template <class Type>
void Image<Type>::setWidth(unsigned int newWidth)
{
	width = newWidth;
}

template <class Type>
float Image<Type>::getExposure()
{
	return exposure;
}

template <class Type>
void Image<Type>::setExposure(float newExposure)
{
	exposure = newExposure;
}

template <class Type>
Type* Image<Type>::getImage()
{
	if(image == NULL)
	{
		image = new Type[height * width * num_of_channels];
	}
	return image;
}

template <class Type>
void Image<Type>::setImage(Type *newHDR) 
{
	image = newHDR;
}

template <class Type>
unsigned int* Image<Type>::getPreviewImage()
{
	if(previewImage == NULL)
	{
		previewImage = new unsigned int[height * width * num_of_channels];
	}
	return previewImage;
}

template <class Type>
void Image<Type>::setPreviewImage(unsigned int *newPreviewImage)
{
	previewImage = newPreviewImage;
}

template <class Type>
void Image<Type>::fill(Type r, Type g, Type b)
{
	Type * hdr_img = getImage();
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

template <class Type>
void Image<Type>::fill(Type g)
{
}

template <class Type>
Image<Type>* Image<Type>::scaled(unsigned int h, unsigned int w)
{
	Image<Type> *img;
	return img;
}
	

template class Image<float>;
template class Image<unsigned char>;