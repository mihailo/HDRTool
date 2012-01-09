#include "ImageAlign.h"


#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../image/Image.h"

ImageAlign::ImageAlign(void)
{
}

ImageAlign::~ImageAlign(void)
{
}




Image* ImageAlign::shiftQImage(Image *in, int dx, int dy)
{
	Image *out=new Image(in->getHeight(), in->getWidth());
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
				out->getHDR()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 0] = in->getHDR()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 0];
				out->getHDR()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 1] = in->getHDR()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 1];
				out->getHDR()[(i + dy) * in->getWidth() * RGB_NUM_OF_CHANNELS 
					+ (j + dx) * RGB_NUM_OF_CHANNELS + 2] = in->getHDR()[i * in->getWidth() * RGB_NUM_OF_CHANNELS + j * RGB_NUM_OF_CHANNELS + 2];
			}
			//inp++;
		}
	}
	return out;
}

void ImageAlign::mtb_alignment(int num_of_image, Image **ImagePtrList, bool **ldr_tiff_input) {
	int width = ImagePtrList[0]->getWidth();
	int height = ImagePtrList[0]->getHeight();
	double quantile = 0.5;
	int noise = 4;
	int shift_bits = qMax((int)floor(log2(qMin(width, height)))-6 , 0);
	logFile("::mtb_alignment: width=%d, height=%d, shift_bits=%d", width, height, shift_bits);

	//these arrays contain the shifts of each image (except the 0-th) wrt the previous one
	int *shiftsX=new int[num_of_image - 1];
	int *shiftsY=new int[num_of_image - 1];

	//find the shitfs
	for (int i = 0; i < num_of_image - 1; i++) 
	{
		mtbalign(ImagePtrList[i], ImagePtrList[i + 1], quantile, noise, shift_bits, shiftsX[i], shiftsY[i]);
	}
	
	logFile("::mtb_alignment: now shifting the images");
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
			logFile("::mtb_alignment: partial cumulativeX=%d, cumulativeY=%d", cumulativeX, cumulativeY);
		}
		logFile("::mtb_alignment: Cumulative shift for image %d = (%d,%d)",i,cumulativeX,cumulativeY);
		Image *shifted=shiftQImage(ImagePtrList[1], cumulativeX, cumulativeY);
		if (ldr_tiff_input[1]) 
		{
			delete [] ImagePtrList[1]->bits();
		}
		delete ImagePtrList[1];
		ImagePtrList.removeAt(1);
		ImagePtrList.append(shifted);
		ldr_tiff_input.removeAt(1);
		ldr_tiff_input.append(false);
	}
	delete shiftsX; 
	delete shiftsY;
}


void ImageAlign::mtbalign(const Image *image1, const Image *image2,
              const double quantile, const int noise, const int shift_bits,
              int &shift_x, int &shift_y)
{
	Image *img1lum=new Image(image1->size(),QImage::Format_Indexed8);
	Image *img2lum=new Image(image2->size(),QImage::Format_Indexed8);
	vector<double> cdf1,cdf2;
	int median1,median2;

	getLum(image1, img1lum, cdf1);
	for(median1 = 0; median1 < 256; median1++) 
		if( cdf1[median1] >= quantile ) break;
	getLum(image2, img2lum, cdf2);
	for(median2 = 0; median2 < 256; median2++) 
		if( cdf2[median2] >= quantile ) break;
	
 	logFile("::mtb_alignment: align::medians, image 1: %d, image 2: %d",median1,median2);
	getExpShift(img1lum, median1, img2lum, median2, noise, shift_bits, shift_x, shift_y);
	delete img1lum; 
	delete img2lum;
	logFile("::mtb_alignment: align::done, final shift is (%d,%d)",shift_x, shift_y);
}

void ImageAlign::getExpShift(const Image *img1, const int median1, 
		 const Image *img2, const int median2, 
		 const int noise, const int shift_bits, 
		 int &shift_x, int &shift_y)
{
	int curr_x = 0;
	int curr_y = 0;
	if( shift_bits > 0) {
		Image img1small = img1->scaled(img1->size()/2);
		Image img2small = img2->scaled(img2->size()/2);
		getExpShift(&img1small,median1, &img2small, median2, noise, shift_bits-1, curr_x, curr_y);
		curr_x *= 2;
		curr_y *= 2;
	}
	
	Image *img1threshold = setbitmap(img1->size());
	Image *img1mask      = setbitmap(img1->size());
	Image *img2threshold = setbitmap(img1->size());
	Image *img2mask      = setbitmap(img1->size());
	setThreshold(img1, median1, noise, img1threshold, img1mask);
	setThreshold(img2, median2, noise, img2threshold, img2mask);
	
	Image *img2th_shifted   = setbitmap(img2->size());
	Image *img2mask_shifted = setbitmap(img2->size());
	Image *diff             = setbitmap(img2->size());

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
 	logFile("::mtb_alignment: getExpShift::Level %d  shift (%d,%d)", shift_bits,shift_x, shift_y);
	return;
}

void ImageAlign::shiftimage(Image *in, const int dx, const int dy, Image *out)
{
	out->fill(0);
	for(int i = 0; i < in->getHeight(); i++) {
		if( (i+dy) < 0 ) continue;
		if( (i+dy) >= in->getHeight()) break;
		const uchar *inp = in->scanLine(i);
		uchar *outp = out->scanLine(i+dy);
		for(int j = 0; j < in->getWidth(); j++) 
		{
			if( (j+dx) >= in->getWidth()) break;
			if( (j+dx) >= 0 ) outp[j+dx] = *inp;
			inp++;
		}
	}
	return;
}

void ImageAlign::setThreshold(const Image *in, const int threshold, const int noise,
			Image *threshold_out, QImage *mask_out)
{
	for(int i = 0; i < in->getHeight(); i++) 
	{
		const uchar *inp = in->scanLine(i);
		uchar *outp = threshold_out->scanLine(i);
		uchar *maskp = mask_out->scanLine(i);
		for(int j = 0; j < in->getWidth(); j++) 
		{
			*outp++ = *inp < threshold ? 0 : 1;
			*maskp++ = (*inp > (threshold-noise)) && (*inp < (threshold+noise)) ? 0 : 1;
			inp++;
		}
	}
	return;
}

void ImageAlign::XORimages(const Image *img1, const Image *mask1, const Image *img2, const Image *mask2, Image *diff)
{
	diff->fill(0);
	for(int i = 0; i < img1->height(); i++) 
	{
		const uchar *p1 = img1->scanLine(i);
		const uchar *p2 = img2->scanLine(i);
		const uchar *m1 = mask1->scanLine(i);
		const uchar *m2 = mask2->scanLine(i);
		uchar *dp = diff->scanLine(i);
		for(int j = 0; j < img1->width(); j++) {
			//*dp++ = xor_t[*p1++][*p2++]*(*m1++)*(*m2++);
			*dp++ = (*p1++ xor *p2++) and *m1++ and *m2++;
		}
	}
	return;
}

long ImageAlign::sumimage(const Image *img)
{
	long ttl  = 0;
	for(int i = 0; i < img->getHeight(); i++) 
	{
		const uchar *p = img->scanLine(i);
		for(int j = 0; j < img->getWidth(); j++) 
			ttl += (long)(*p++);
	}
	return ttl;
}

void ImageAlign::getLum(const Image *in, Image *out, vector<double> &cdf)
{
	vector<long> hist;
	hist.resize(256);
	unsigned int i;
	for(i = 0; i < 256; i++) hist[i] = 0;

	cdf.resize(256);
	
	QVector<QRgb> graycolortable;
	
	for(i = 0; i < 256; i++)
		graycolortable.append(qRgb(i,i,i));
	out->setColorTable(graycolortable);
	
	for(int i = 0; i < in->getHeight(); i++) 
	{
		QRgb *inl = (QRgb *)in->scanLine(i);
		uchar *outl = out->scanLine(i);
		for(int j = 0; j < in->getWidth(); j++) {
			uint v = qGray(*inl);
			hist[v] = hist[v] + 1;
			inl++;
			*outl++ = v;
		}
	}
	double w = 1/((double)(in->getHeight()*in->getWidth()));
	cdf[0] = w*hist[0];
	for(int i = 1; i < 256; i++) cdf[i] = cdf[i-1] + w*hist[i];
	return;
}

//it's up to the caller to "delete" the pointer to free the mem!
Image* ImageAlign::setbitmap(const QSize size)
{
	Image *img = new QImage(size,QImage::Format_Indexed8);
	
	QVector<QRgb> binaryColorTable;
	binaryColorTable.append(qRgb(0,0,0));
	binaryColorTable.append(qRgb(255,255,255));
	img->setColorTable(binaryColorTable);
	return img;
}
