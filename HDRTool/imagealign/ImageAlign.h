#ifndef IMAGE_ALIGN_H_
#define IMAGE_ALIGN_H_

#include "../image/Image.h"
#include <vector>

class ImageAlign
{
public:
	ImageAlign(void);
	~ImageAlign(void);


	void mtb_alignment(int num_of_image, Image **ImagePtrList, bool **ldr_tiff_input);

	Image* shiftQImage(Image *in, int dx, int dy);

	///////// private functions
	void mtbalign(const Image *image1, const Image *image2,
		const double quantile, const int noise, const int shift_bits,
		int &shift_x, int &shift_y);

	void getExpShift(const Image *img1, const int median1, 
		const Image *img2, const int median2, 
		const int noise, const int shift_bits, 
		int &shift_x, int &shift_y);

	void shiftimage(Image *in, const int dx, const int dy, Image *out);

	long sumimage(const Image *img);

	void XORimages(const Image *img1, const Image *mask1, const Image *img2, const Image *mask2, Image *diff);

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
	void setThreshold(const Image *in, const int threshold, const int noise, 
		Image *out, Image *mask);

	void getLum(const Image *in, Image *out, vector<double> &cdf);

	/**
	* setbitmap allocates mem for a QImage pointer
	* it's up to the caller to "delete" the pointer to free the mem.
	*
	* \param size the image size \n
	* \return the pointer for which mem will be allocated
	*/
	Image* setbitmap(const QSize size);
};

#endif
