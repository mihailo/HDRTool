#ifndef _IMAGESCALE_H_
#define _IMAGESCALE_H_

#include "../image/Image.h"
#include "Matrix.h"

struct QImageScaleInfo {
        int *xpoints;
        unsigned int **ypoints;
        int *xapoints, *yapoints;
        int xup_yup;
    };
 

class ImageScale
{
private:
public:
	Image<unsigned char>* scaled(Image<unsigned char>* image, unsigned int h, unsigned int w)const;
	Image<unsigned char>* smoothScaled(Image<unsigned char>* source, int w, int h);
	Image<unsigned char>* smoothScaleImage(Image<unsigned char>* img);
	Image<unsigned char>* transformed(const Transform &matrix) const;

	Image<unsigned char>* qSmoothScaleImage(Image<unsigned char>* src, int dw, int dh);
	QImageScaleInfo* qimageCalcScaleInfo(Image<unsigned char> *img, int sw, int sh, int dw, int dh, char aa);


	int* qimageCalcApoints(int s, int d, int up);
	int* qimageCalcXPoints(int sw, int dw);
	unsigned int** qimageCalcYPoints(unsigned int *src, int sw, int sh, int dh);
};


#endif
