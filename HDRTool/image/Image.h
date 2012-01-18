#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "Data.h"

template <class Type>
class Image
{
private:
	unsigned int num_of_channels;

	unsigned int height;
	unsigned int width;

	float exposure;

	Type *image;
	unsigned int *previewImage;

public:
	Image(unsigned int num_channels);
	Image(unsigned int num_channels, unsigned int h, unsigned int w);
	~Image(void);

	
	unsigned int getHeight();
	void setHeight(unsigned int newHeight);
	
	unsigned int getWidth();
	void setWidth(unsigned int newWidth);

	float getExposure();
	void setExposure(float newExposure);

	Type* getImage();
	void setImage(Type *image);

	unsigned int* getPreviewImage();
	void setPreviewImage(unsigned int *newPreviewImage);

	Image* scaled(unsigned int h, unsigned int w);
	void fill(Type r, Type g, Type b);
	void fill(Type g);
};

#endif