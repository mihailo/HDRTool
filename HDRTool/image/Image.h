#pragma once
#include "Data.h"

class Image
{
private:
	unsigned int height;
	unsigned int width;

	float exposure;

	float *image;
	guchar *previewImage;

public:
	Image(void);
	~Image(void);

	
	unsigned int getHeight();
	void setHeight(unsigned int newHeight);
	
	unsigned int getWidth();
	void setWidth(unsigned int newWidth);

	float getExposure();
	void setExposure(float newExposure);

	float* getHDR();
	void setHDR(float *newHDR);

	guchar* getPreviewImage();
	void setPreviewImage(guchar *newPreviewImage);
};
