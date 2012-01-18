#ifndef IMAGE_ALIGN_H_
#define IMAGE_ALIGN_H_

#include "../image/Image.h"

class ImageAlign
{
public:
	ImageAlign(void);
	~ImageAlign(void);


	void mtb_alignment(int num_of_image, Image<unsigned char> **ImagePtrList, bool **ldr_tiff_input);

	Image<unsigned char>* shiftQImage(Image<unsigned char> *in, int dx, int dy);

	///////// private functions
	void mtbalign(Image<unsigned char> *image1, Image<unsigned char> *image2,
		const double quantile, const int noise, const int shift_bits,
		int &shift_x, int &shift_y);

	void getExpShift(Image<unsigned char> *img1, const int median1, 
		Image<unsigned char> *img2, const int median2, 
		const int noise, const int shift_bits, 
		int &shift_x, int &shift_y);

	void shiftimage(Image<unsigned char> *in, const int dx, const int dy, Image<unsigned char> *out);

	long sumimage(Image<unsigned char> *img);

	void XORimages(Image<unsigned char> *img1, Image<unsigned char> *mask1, Image<unsigned char> *img2, Image<unsigned char> *mask2, Image<unsigned char> *diff);

	/**
	* setThreshold gets the data from the input image and creates the threshold and mask images.
	* those should be bitmap (0,1 valued) with depth()=1 but instead they are like grayscale
	* indexed images, with depth()=8. This waste of space happens because the grayscale
	* images are easier to deal with (no bit shifts).
	*
	* \param *in source data image \n
	* \param threshold the threshold value \n
	* \param noise \n
	* \param *out the output image (8bit) \n
	* \param *mask the noise mask image \n	
	*/
	void setThreshold(Image<unsigned char> *in, const int threshold, const int noise, 
		Image<unsigned char> *out, Image<unsigned char> *mask);

	void getLum(Image<unsigned char> *in, Image<unsigned char> *out, double *cdf);

	/**
	* setbitmap allocates mem for a QImage pointer
	* it's up to the caller to "delete" the pointer to free the mem.
	*
	* \param size the image size \n
	* \return the pointer for which mem will be allocated
	*/
	Image<unsigned char>* setbitmap(const unsigned int height, const unsigned int width);

	//void maxInt(int a, int b);
	//void minInt(int a, int b);
};

#endif
