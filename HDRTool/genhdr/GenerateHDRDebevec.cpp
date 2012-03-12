#include "GenerateHDRDebevec.h"

#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"
#include "../utils/timer/hr_time.h"


GenerateHDRDebevec::GenerateHDRDebevec()
{
	core = new OpenCLCore(this, "genhdr/generate_HDR_debevec.cl", "generate_hdri_pixels");
}

GenerateHDRDebevec::~GenerateHDRDebevec()
{
	delete[] i_lower;
	delete[] i_upper;
	delete core;
}

void GenerateHDRDebevec::generateHDR(Image<float> *img, float *arrayofexptime, float *Ir, float *Ig, float* Ib, float *W, int M, int numimg, unsigned char *ldr_img)
{
	image = img;
	

	array_of_exp_time = arrayofexptime;
	ir = Ir;
	ig = Ig;
	ib = Ib;
	w = W;
	m = M;
	num_ldr = numimg;
	ldr = ldr_img;

	localWorkSize[0] = BLOCK_SIZE;
	localWorkSize[1] = BLOCK_SIZE;
	
	//round values on upper value
	logFile("%d %d \n", image->getHeight(), image->getWidth());
	globalWorkSize[0] = roundUp(BLOCK_SIZE, image->getHeight());
	globalWorkSize[1] = roundUp(BLOCK_SIZE, image->getWidth());

	min_m = 0;
	int mi;
	for(mi = 0; mi < m; mi++) // M == 256
	{
		if(w[mi] > 0)
		{
			min_m = mi;
			break;
		}
	}
	max_m = m - 1;
	for(mi = m - 1; mi >= 0; mi--)
	{
		if(w[mi] > 0)
		{
			max_m = mi;
			break;
		}
	}

	printf("min_m = %d max_m = %d \n", min_m, max_m);

	// --- anti ghosting: for each image i, find images with
	// the immediately higher and lower exposure times
	i_lower = new int[num_ldr];
	i_upper = new int[num_ldr];
	int i;
	for(i = 0; i < num_ldr; i++)
	{
		i_lower[i]=-1;
		i_upper[i]=-1;
		float ti = array_of_exp_time[i];
		float ti_upper = +1e6;
		float ti_lower = -1e6;

		int j;
		for(j = 0; j < num_ldr; j++)
			if(i != j)
			{
				if(array_of_exp_time[j] > ti && array_of_exp_time[j] < ti_upper)
				{
					ti_upper = array_of_exp_time[j];
					i_upper[i] = j;
				}
				if(array_of_exp_time[j] < ti && array_of_exp_time[j] > ti_lower)
				{
					ti_lower = array_of_exp_time[j];
					i_lower[i] = j;
				}
			}
		if(i_lower[i] == -1)
			i_lower[i] = i;
		if( i_upper[i] == -1 )
			i_upper[i] = i;
	}

	for(int x = 0; x<3; x++)
	{
		printf("%d %d ", i_lower[x], i_upper[x]);
	}
	printf("\n");

	/*CStopWatch timer;
	timer.startTimer();
	calcHDRCPU();
	timer.stopTimer();
	logFile("genHDRCPU,calc_time, , ,%f, \n", timer.getElapsedTime());*/
	
	//paralel version
	core->runComputeUnit();
}

void GenerateHDRDebevec::calcHDRCPU()
{
	int width = image->getWidth();
	int height = image->getHeight();
	
	
	for(int j = 0; j < width * height; j++){
		float sumR = 0.0f;
		float sumG = 0.0f;
		float sumB = 0.0f;

		float divR = 0.0f;
		float divG = 0.0f;
		float divB = 0.0f;

		float maxti = -1e6f;
		float minti = +1e6f;

		int index_for_whiteR = -1;
		int index_for_whiteG = -1;
		int index_for_whiteB = -1;

		int index_for_blackR = -1;
		int index_for_blackG = -1;
		int index_for_blackB = -1;

		// for all exposures
		for(int i = 0; i < num_ldr; i++)
		{
			//pick the 3 channel values + alpha
			int mR = ldr[i * width * height * 3 + j * 3 + 0];
			int mG = ldr[i * width * height * 3 + j * 3 + 1];
			int mB = ldr[i * width * height * 3 + j * 3 + 2];
			int mA = 255;// posto ga posle koristi !!!! int mA = qAlpha(* ( (QRgb*)( (listLDR->at(i) )->bits() ) + j ) );
			
			float ti = array_of_exp_time[i];
			// --- anti ghosting: monotonous increase in time should result
			// in monotonous increase in intensity; make forward and
			// backward check, ignore value if condition not satisfied
			int R_lower = ldr[i_lower[i] * width * height * 3 + j * 3 + 0];//qRed  (* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int R_upper = ldr[i_upper[i] * width * height * 3 + j * 3 + 0];//qRed  (* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );
			int G_lower = ldr[i_lower[i] * width * height * 3 + j * 3 + 1];//qGreen(* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int G_upper = ldr[i_upper[i] * width * height * 3 + j * 3 + 1];//qGreen(* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );
			int B_lower = ldr[i_lower[i] * width * height * 3 + j * 3 + 2];//qBlue (* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int B_upper = ldr[i_upper[i] * width * height * 3 + j * 3 + 2];//qBlue (* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );

			//if at least one of the color channel's values are in the bright "not-trusted zone" and we have min exposure time
			if ((mR > max_m || mG > max_m || mB > max_m) && (ti < minti))
			{
				//update the indexes_for_whiteRGB, minti
				index_for_whiteR = mR;
				index_for_whiteG = mG;
				index_for_whiteB = mB;
				minti = ti;
				//continue;
			}

			//if at least one of the color channel's values are in the dim "not-trusted zone" and we have max exposure time
			if ((mR < min_m || mG < min_m || mB < min_m) && (ti > maxti))
			{
				//update the indexes_for_blackRGB, maxti
				index_for_blackR = mR;
				index_for_blackG = mG;
				index_for_blackB = mB;
				maxti = ti;
				//continue;
			}

			//The OR condition seems to be required in order not to have large areas of "invalid" color, need to investigate more.
			if (R_lower > mR || G_lower > mG || B_lower > mB)
			{
				//update the indexes_for_whiteRGB, minti
				index_for_whiteR = mR;
				index_for_whiteG = mG;
				index_for_whiteB = mB;
				minti = ti;
				continue;
			}
			if (R_upper<mR || G_upper<mG || B_upper<mB)
			{
				//update the indexes_for_blackRGB, maxti
				index_for_blackR = mR;
				index_for_blackG = mG;
				index_for_blackB = mB;
				maxti = ti;
				continue;
			}
			// mA assumed to handle de-ghosting masks
			// mA values assumed to be in [0, 255]
			// mA=0 assummed to mean that the pixel should be excluded
			float w_average = (float)mA * (w[mR] + w[mG] + w[mB]) / (3.0f * 255.0f);
			sumR += w_average * ir[mR] / (float)ti;
			divR += w_average;
			sumG += w_average * ig[mG] / (float)ti;
			divG += w_average;
			sumB += w_average * ib[mB] / (float)ti;
			divB += w_average;
		} //END for all the exposures

		if(divR == 0.0f || divG == 0.0f || divB == 0.0f)
		{
			if (maxti > -1e6f)
			{
				sumR = ir[index_for_blackR] / (float)maxti;
				sumG = ig[index_for_blackG] / (float)maxti;
				sumB = ib[index_for_blackB] / (float)maxti;
				divR = divG = divB = 1.0f;
			}
			else
				if (minti < +1e6f)
				{
					sumR = ir[index_for_whiteR] / (float)minti;
					sumG = ig[index_for_whiteG] / (float)minti;
					sumB = ib[index_for_whiteB] / (float)minti;
					divR = divG = divB = 1.0f;
				}
		}

		if(divR != 0.0f && divG != 0.0f && divB != 0.0f)
		{
			image->getImage()[j * 3 + 0] = sumR/divR;
			image->getImage()[j * 3 + 1] = sumG/divG;
			image->getImage()[j * 3 + 2] = sumB/divB;

			//cl_hdrpic[j * 3 + 0] = (unsigned char)clamps(cl_hdr[j * 3 + 0], 0.0f, 255.0f);
			//cl_hdrpic[j * 3 + 1] = (unsigned char)clamps(cl_hdr[j * 3 + 1], 0.0f, 255.0f);
			//cl_hdrpic[j * 3 + 2] = (unsigned char)clamps(cl_hdr[j * 3 + 2], 0.0f, 255.0f);
		}
		else
		{
			//we shouldn't be here anyway...
			//printf("jel ulazim ovde");
			//Rout_cuda[j] = 0.0f;
			//Gout_cuda[j] = 0.0f;
			//Bout_cuda[j] = 0.0f;

			image->getImage()[j * 3 + 0] = 0.0f;
			image->getImage()[j * 3 + 1] = 0.0f;
			image->getImage()[j * 3 + 2] = 0.0f;

			//cl_hdrpic[j * 3 + 0] = (unsigned char)cl_hdr[j * 3 + 0];
			//cl_hdrpic[j * 3 + 1] = (unsigned char)cl_hdr[j * 3 + 1];
			//cl_hdrpic[j * 3 + 2] = (unsigned char)cl_hdr[j * 3 + 2];
		}
	}
}

void GenerateHDRDebevec::allocateOpenCLMemory()
{
	cl_int ciErr1, ciErr2;


	// Allocate the OpenCL buffer memory objects for source and result on the device MEM
	unsigned int size_ldr = num_ldr * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS * sizeof(unsigned char);
	cl_ldr_img = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_ldr, NULL, &ciErr1);

	unsigned int size_array = num_ldr * sizeof(float);
	cl_array_of_exp_time = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_array, NULL, &ciErr2);
	ciErr1 |= ciErr2;

	unsigned int size_i = num_ldr * sizeof(int);
	cl_i_lower = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_i, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_i_upper = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_i, NULL, &ciErr2);
	ciErr1 |= ciErr2;

	unsigned int size_w = m * sizeof(float);
	cl_w = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_w, NULL, &ciErr2);
	ciErr1 |= ciErr2;

	unsigned int size_Irgb = m * sizeof(cl_float);
	cl_ir = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_Irgb, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_ig = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_Irgb, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	cl_ib = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_Irgb, NULL, &ciErr2);
	ciErr1 |= ciErr2;
	
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	cl_hdr = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size, NULL, &ciErr2);
	ciErr1 |= ciErr2;

	unsigned int size_hdrpic = sizeof(unsigned char) * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS;
	cl_hdrpic = clCreateBuffer(core->getGPUContext(), CL_MEM_READ_WRITE, 
		size_hdrpic, NULL, &ciErr2);
	ciErr1 |= ciErr2;

    logFile("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clCreateBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void GenerateHDRDebevec::setInputDataToOpenCLMemory()
{
	int height = image->getHeight();
	int width = image->getWidth();
	int N = num_ldr;
	
	cl_int ciErr1;				
	// Set the Argument values
	ciErr1 = clSetKernelArg(core->getOpenCLKernel(), 0, 
		sizeof(cl_int), (void*)&width);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 1, 
		sizeof(cl_int), (void*)&height);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 2, 
		sizeof(cl_int), (void*)&N);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 3, 
		sizeof(cl_mem), (void*)&cl_ldr_img);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 4, 
		sizeof(cl_mem), (void*)&cl_array_of_exp_time);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 5, 
		sizeof(cl_mem), (void*)&cl_i_lower);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 6, 
		sizeof(cl_mem), (void*)&cl_i_upper);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 7, 
		sizeof(cl_int), (void*)&max_m);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 8, 
		sizeof(cl_int), (void*)&min_m);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 9, 
		sizeof(cl_mem), (void*)&cl_w);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 10, 
		sizeof(cl_mem), (void*)&cl_ir);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 11, 
		sizeof(cl_mem), (void*)&cl_ig);
	ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 12, 
		sizeof(cl_mem), (void*)&cl_ib);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 13, 
		sizeof(cl_mem), (void*)&cl_hdr);
    ciErr1 |= clSetKernelArg(core->getOpenCLKernel(), 14, 
		sizeof(cl_mem), (void*)&cl_hdrpic);
	
	logFile("clSetKernelArg 0 - 14...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clSetKernelArg, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

	clFinish(core->getCqCommandQueue());
	CStopWatch timer;
	timer.startTimer();

    // Asynchronous write of data to GPU device
	unsigned int size_ldr = num_ldr * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS * sizeof(unsigned char);
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ldr_img, CL_TRUE, 0, 
		size_ldr, ldr, 0, NULL, NULL);
	unsigned int size_array = num_ldr * sizeof(float);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_array_of_exp_time, CL_TRUE, 0, 
		size_array, array_of_exp_time, 0, NULL, NULL);

	unsigned int size_w = m * sizeof(cl_float);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_w, CL_TRUE, 0, 
		size_w, w, 0, NULL, NULL);

	unsigned int size_i = num_ldr * sizeof(int);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_i_lower, CL_TRUE, 0, 
		size_i, i_lower, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_i_upper, CL_TRUE, 0, 
		size_i, i_upper, 0, NULL, NULL);
    unsigned int size_Irgb = m * sizeof(cl_float);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ir, CL_TRUE, 0, 
		size_Irgb, ir, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ig, CL_TRUE, 0, 
		size_Irgb, ig, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ib, CL_TRUE, 0, 
		size_Irgb, ib, 0, NULL, NULL);

	clFinish(core->getCqCommandQueue());
	timer.stopTimer();
	logFile("gpuDEBEVEC,data_in,%d,%d,%f, \n", height, width, timer.getElapsedTime());

	logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void GenerateHDRDebevec::getDataFromOpenCLMemory()
{
	clFinish(core->getCqCommandQueue());
	CStopWatch timer;
	timer.startTimer();
	
	// Synchronous/blocking read of results, and check accumulated errors
	cl_int ciErr1;			// Error code var
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_hdr, CL_TRUE, 0, 
		size, image->getImage(), 0, NULL, NULL);
	
	unsigned int size_hdrpic = sizeof(unsigned char) * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS;
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_hdrpic, CL_TRUE, 0, 
		size_hdrpic, image->getPreviewImage(), 0, NULL, NULL);	

	clFinish(core->getCqCommandQueue());
	timer.stopTimer();
	logFile("gpuDEBEVEC,data_out,%d,%d,%f, \n", image->getHeight(), image->getWidth(), timer.getElapsedTime());

    logFile("clEnqueueReadBuffer ...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void GenerateHDRDebevec::clearDeviceMemory()
{
	clReleaseMemObject(cl_ldr_img);
	clReleaseMemObject(cl_array_of_exp_time);
	clReleaseMemObject(cl_i_lower);
	clReleaseMemObject(cl_i_upper);
	clReleaseMemObject(cl_w);
	clReleaseMemObject(cl_ir);
	clReleaseMemObject(cl_ig);
	clReleaseMemObject(cl_ib);
	clReleaseMemObject(cl_hdr);
	clReleaseMemObject(cl_hdrpic);
}

int GenerateHDRDebevec::getNumOfDim()
{
	return 2;
}

size_t* GenerateHDRDebevec::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* GenerateHDRDebevec::getSzLocalWorkSize()
{
	return localWorkSize;
}