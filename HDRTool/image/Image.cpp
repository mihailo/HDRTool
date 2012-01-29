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
	Type * hdr_img = getImage();
	unsigned int x,y;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			hdr_img[y * width + x] = g;
		}
	}
}

#define AVERAGE(a, b)   (unsigned char)( ((a) + (b)) >> 1 )

template <class Type>
Image<unsigned char>* Image<Type>::scaled2()
{
	Image<unsigned char> *img2 = new Image<unsigned char>(num_of_channels, height / 2, width / 2);
	//img2 = new Image<unsigned char>(num_of_channels, height / 2, width / 2);
	
	unsigned int x, y, x2, y2;
	unsigned char p, q;
	
	for (y = 0; y < img2->getHeight(); y++) 
	{
		y2 = 2 * y;
		for (x = 0; x < img2->getWidth(); x++)
		{
			x2 = 2 * x;
			for(unsigned int ch = 0; ch < num_of_channels; ch++)
			{
				p = AVERAGE((unsigned char)image[y2 * width * num_of_channels + x2 * num_of_channels + ch], (unsigned char)image[y2 * width * num_of_channels + (x2 + 1) * num_of_channels + ch]);
				q = AVERAGE((unsigned char)image[(y2 + 1) * width * num_of_channels + x2 * num_of_channels + ch], (unsigned char)image[(y2 + 1) * width * num_of_channels + (x2 + 1) * num_of_channels + ch]);
				img2->getImage()[y * img2->getWidth() * num_of_channels + x * num_of_channels + ch] = AVERAGE(p, q);
				//printf("%d ", AVERAGE(p, q));
			}
		} 
		//printf("\n");
	} 

	return img2;
}

template class Image<float>;
template class Image<unsigned char>;