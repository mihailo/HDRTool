#include "ImageAlign.h"

#include <math.h>
#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../image/Image.h"
#include <iso646.h>

ImageAlign::ImageAlign(void)
{
}

ImageAlign::~ImageAlign(void)
{
}




Image<unsigned char>* ImageAlign::shiftQImage(Image<unsigned char> *in, int dx, int dy)
{
	Image<unsigned char> *out=new Image<unsigned char>(3, in->getHeight(), in->getWidth());
	out->fill(0,0,0); //transparent black
	unsigned int i;
	for(i = 0; i < in->getHeight(); i++) 
	{
		if((i + dy) < 0) continue;
		if((i + dy) >= in->getHeight()) break;
		//QRgb *inp = (QRgb*)in->scanLine(i);
		//QRgb *outp = (QRgb*)out->scanLine(i+dy);
		unsigned int j;
		for(j = 0; j < in->getWidth(); j++) 
		{
			if( (j+dx) >= in->getWidth()) break;
			if( (j+dx) >= 0 ) 
			{
				//outp[j+dx] = *inp;
				out->getImage()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 0] = in->getImage()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 0];
				out->getImage()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 1] = in->getImage()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 1];
				out->getImage()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 2] = in->getImage()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 2];
			}
			//inp++;
		}
	}
	return out;
}

void ImageAlign::mtb_alignment(int num_of_image, Image<unsigned char> **ImagePtrList, bool *ldr_tiff_input) 
{
	int width = ImagePtrList[0]->getWidth();
	int height = ImagePtrList[0]->getHeight();
	double quantile = 0.5;
	int noise = 4;
	int shift_bits = max((int)floor(log((double)min(width, height)) / log((double)2)) - 6 , 0); //calculate log for base 2 log( n ) / log( 2 ); 
	logFile("::mtb_alignment: width=%d, height=%d, shift_bits=%d\n", width, height, shift_bits);

	//these arrays contain the shifts of each image (except the 0-th) wrt the previous one
	int *shiftsX=new int[num_of_image - 1];
	int *shiftsY=new int[num_of_image - 1];

	//find the shitfs
	for (int i = 0; i < num_of_image - 1; i++) 
	{
		mtbalign(ImagePtrList[i], ImagePtrList[i + 1], quantile, noise, shift_bits, shiftsX[i], shiftsY[i]);
	}
	
	logFile("::mtb_alignment: now shifting the images\n");
	int originalsize = num_of_image; //ImagePtrList.size();
	//shift the images (apply the shifts starting from the second (index=1))
	for (int i = 1; i < originalsize; i++) 
	{
		int cumulativeX = 0;
		int cumulativeY = 0;
		//gather all the x,y shifts until you reach the first image
		for (int j = i - 1; j >= 0; j--) 
		{
			cumulativeX += shiftsX[j];
			cumulativeY += shiftsY[j];
			logFile("::mtb_alignment: partial cumulativeX=%d, cumulativeY=%d\n", cumulativeX, cumulativeY);
		}
		logFile("::mtb_alignment: Cumulative shift for image %d = (%d,%d)\n",i,cumulativeX,cumulativeY);
		Image<unsigned char> *shifted=shiftQImage(ImagePtrList[1], cumulativeX, cumulativeY);
		/*if (ldr_tiff_input[1]) 
		{
			delete [] ImagePtrList[1]->bits();
		}*/
		//result
		/*delete ImagePtrList[1];
		ImagePtrList.removeAt(1);
		ImagePtrList.append(shifted);
		ldr_tiff_input.removeAt(1);
		ldr_tiff_input.append(false);*/
	}
	delete shiftsX; 
	delete shiftsY;
}


void ImageAlign::mtbalign(Image<unsigned char> *image1, Image<unsigned char> *image2,
              const double quantile, const int noise, const int shift_bits,
              int &shift_x, int &shift_y)
{
	Image<unsigned char> *img1lum=new Image<unsigned char>(1, image1->getHeight(), image1->getWidth());//QImage::Format_Indexed8);
	Image<unsigned char> *img2lum=new Image<unsigned char>(1, image2->getHeight(), image2->getWidth());//QImage::Format_Indexed8);
	double cdf1[256], cdf2[256];
	int median1,median2;

	getLum(image1, img1lum, cdf1);
	for(median1 = 0; median1 < 256; median1++) 
		if( cdf1[median1] >= quantile ) break;
	getLum(image2, img2lum, cdf2);
	for(median2 = 0; median2 < 256; median2++) 
		if( cdf2[median2] >= quantile ) break;
	
 	logFile("::mtb_alignment: align::medians, image 1: %d, image 2: %d\n",median1,median2);
	getExpShift(img1lum, median1, img2lum, median2, noise, shift_bits, shift_x, shift_y);
	delete img1lum; 
	delete img2lum;
	logFile("::mtb_alignment: align::done, final shift is (%d,%d)\n",shift_x, shift_y);
}

void ImageAlign::getExpShift(Image<unsigned char> *img1, const int median1, 
		 Image<unsigned char> *img2, const int median2, 
		 const int noise, const int shift_bits, 
		 int &shift_x, int &shift_y)
{
	int curr_x = 0;
	int curr_y = 0;
	if( shift_bits > 0) {
		Image<unsigned char> *img1small = img1->scaled2();
		Image<unsigned char> *img2small = img2->scaled2();
		getExpShift(img1small, median1, img2small, median2, noise, shift_bits-1, curr_x, curr_y);
		curr_x *= 2;
		curr_y *= 2;
	}
	
	Image<unsigned char> *img1threshold = setbitmap(img1->getHeight(), img1->getWidth());
	Image<unsigned char> *img1mask      = setbitmap(img1->getHeight(), img1->getWidth());
	Image<unsigned char> *img2threshold = setbitmap(img1->getHeight(), img1->getWidth());
	Image<unsigned char> *img2mask      = setbitmap(img1->getHeight(), img1->getWidth());
	setThreshold(img1, median1, noise, img1threshold, img1mask);
	setThreshold(img2, median2, noise, img2threshold, img2mask);
	
	Image<unsigned char> *img2th_shifted   = setbitmap(img2->getHeight(), img2->getWidth());
	Image<unsigned char> *img2mask_shifted = setbitmap(img2->getHeight(), img2->getWidth());
	Image<unsigned char> *diff             = setbitmap(img2->getHeight(), img2->getWidth());

	int minerr = img1->getWidth() * img2->getHeight();
	for(int i = -1; i <= 1; i++) 
	{
		for(int j = -1; j <= 1; j++) 
		{
			int dx = curr_x + i;
			int dy = curr_y + j;
			shiftimage(img2threshold, dx, dy, img2th_shifted);
			shiftimage(img2mask, dx, dy, img2mask_shifted);
			XORimages(img1threshold, img1mask, img2th_shifted, img2mask_shifted, diff);
			long err = sumimage(diff);
			if( err < minerr ) {
				minerr = err;
				shift_x = dx;
				shift_y = dy;
			}
		}
	}

	delete img1threshold; 
	delete img1mask; 
	delete img2threshold; 
	delete img2mask;
	delete img2th_shifted; 
	delete img2mask_shifted; 
	delete diff;
 	logFile("::mtb_alignment: getExpShift::Level %d  shift (%d,%d)\n", shift_bits,shift_x, shift_y);
	return;
}

void ImageAlign::shiftimage(Image<unsigned char> *in, const int dx, const int dy, Image<unsigned char> *out)
{
	//obe su kao grayscale
	out->fill(0);
	unsigned int i;
	for(i = 0; i < in->getHeight(); i++) {
		if( (i+dy) < 0 ) continue;
		if( (i+dy) >= in->getHeight()) break;
		//const unsigned char *inp = in->scanLine(i);
		//unsigned char *outp = out->scanLine(i+dy);
		unsigned int j;
		for(j = 0; j < in->getWidth(); j++) 
		{
			if( (j+dx) >= in->getWidth()) break;
			if( (j+dx) >= 0 ) out->getImage()[i * in->getWidth() + j + dx] = in->getImage()[i * in->getWidth() + j]; //outp[j+dx] = *inp;
			//inp++;
		}
	}
	return;
}

void ImageAlign::setThreshold(Image<unsigned char> *in, const int threshold, const int noise,
			Image<unsigned char> *threshold_out, Image<unsigned char> *mask_out)
{
	unsigned int i;
	for(i = 0; i < in->getHeight(); i++) 
	{
		//const unsigned char *inp = in->scanLine(i);
		//unsigned char *outp = threshold_out->scanLine(i);
		//unsigned char *maskp = mask_out->scanLine(i);
		unsigned int j;
		for(j = 0; j < in->getWidth(); j++) 
		{
			threshold_out->getImage()[i * threshold_out->getWidth() + j] = 
				in->getImage()[i * threshold_out->getWidth() + j] < threshold ? 0 : 1;//*outp++ = *inp < threshold ? 0 : 1;
			mask_out->getImage()[i * threshold_out->getWidth() + j] = 
				(in->getImage()[i * threshold_out->getWidth() + j] > (threshold-noise)) && (in->getImage()[i * threshold_out->getWidth() + j] < (threshold+noise)) ? 0 : 1;
			//inp++;
		}
	}
	return;
}

void ImageAlign::XORimages(Image<unsigned char> *img1, Image<unsigned char> *mask1, Image<unsigned char> *img2, Image<unsigned char> *mask2, Image<unsigned char> *diff)
{
	diff->fill(0);
	unsigned int i;
	for(i = 0; i < img1->getHeight(); i++) 
	{
		//const unsigned char *p1 = img1->scanLine(i);
		//const unsigned char *p2 = img2->scanLine(i);
		//const unsigned char *m1 = mask1->scanLine(i);
		//const unsigned char *m2 = mask2->scanLine(i);
		//unsigned char *dp = diff->scanLine(i);
		unsigned int j;
		for(j = 0; j < img1->getWidth(); j++) {
			//*dp++ = xor_t[*p1++][*p2++]*(*m1++)*(*m2++);
			diff->getImage()[i * img1->getWidth() + j] = (img1->getImage()[i * img1->getWidth() + j] xor img2->getImage()[i * img1->getWidth() + j]) 
				and mask1->getImage()[i * img1->getWidth() + j] and mask2->getImage()[i * img1->getWidth() + j]; //*dp++ = (*p1++ xor *p2++) and *m1++ and *m2++;
		}
	}
	return;
}

long ImageAlign::sumimage(Image<unsigned char> *img)
{
	long ttl  = 0;
	unsigned int i;
	for(i = 0; i < img->getHeight(); i++) 
	{
		//const unsigned char *p = img->scanLine(i);
		unsigned int j;
		for(j = 0; j < img->getWidth(); j++) 
			ttl += (long)(img->getImage()[i * img->getWidth() + j]);
	}
	return ttl;
}

void ImageAlign::getLum(Image<unsigned char> *in, Image<unsigned char> *out, double *cdf)
{
	//in je rgb, out je gray
	long hist[256];
	unsigned int i, j;
	for(i = 0; i < 256; i++) hist[i] = 0;
	
	//QVector<QRgb> graycolortable;
	
	//for(i = 0; i < 256; i++)
	//	graycolortable.append(qRgb(i,i,i));
	//out->setColorTable(graycolortable);
	
	for(i = 0; i < in->getHeight(); i++) 
	{
		//QRgb *inl = (QRgb *)in->scanLine(i);
		//unsigned char *outl = out->scanLine(i);
		for(j = 0; j < in->getWidth(); j++) {
			unsigned int v = (in->getImage()[(i * in->getWidth() + j) * 3 + 0] * 54 
				+ in->getImage()[(i * in->getWidth() + j) * 3 + 1] * 183
				+ in->getImage()[(i * in->getWidth() + j) * 3 + 2] * 19) / 256;//qGray(*inl); // preracunava u sivo!!!! // pre bi bilo da je unsigned char
			hist[v] = hist[v] + 1;
			//inl++;
			out->getImage()[i * out->getWidth() + j] = v;
		}
	}
	double w = 1/((double)(in->getHeight()*in->getWidth()));
	cdf[0] = w*hist[0];
	for(i = 1; i < 256; i++) cdf[i] = cdf[i-1] + w*hist[i];
	return;
}

//it's up to the caller to "delete" the pointer to free the mem!
Image<unsigned char>* ImageAlign::setbitmap(const unsigned int height, const unsigned int width)
{
	Image<unsigned char> *img = new Image<unsigned char>(3, height, width);
	//ili 0 ili 255 treba provaliti gde se poziva!!!
	//QVector<QRgb> binaryColorTable;
	//binaryColorTable.append(qRgb(0,0,0));
	//binaryColorTable.append(qRgb(255,255,255));
	//img->setColorTable(binaryColorTable);
	return img;
}
