#include "GenerateHDRDebevec.h"

#include "../utils/Log.h"
#include "../utils/Consts.h"
#include "../utils/clUtil/OpenCLUtil.h"


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

	core->runComputeUnit();
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

	unsigned int size_Irgb = m * sizeof(float);
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

    // Asynchronous write of data to GPU device
	unsigned int size_ldr = num_ldr * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS * sizeof(unsigned char);
	ciErr1 = clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ldr_img, CL_TRUE, 0, 
		size_ldr, ldr, 0, NULL, NULL);
	unsigned int size_array = num_ldr * sizeof(float);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_array_of_exp_time, CL_TRUE, 0, 
		size_array, array_of_exp_time, 0, NULL, NULL);
	unsigned int size_i = num_ldr * sizeof(int);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_i_lower, CL_TRUE, 0, 
		size_i, i_lower, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_i_upper, CL_TRUE, 0, 
		size_i, i_upper, 0, NULL, NULL);
    unsigned int size_Irgb = m * sizeof(float);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ir, CL_TRUE, 0, 
		size_Irgb, ir, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ig, CL_TRUE, 0, 
		size_Irgb, ig, 0, NULL, NULL);
	ciErr1 |= clEnqueueWriteBuffer(core->getCqCommandQueue(), cl_ib, CL_TRUE, 0, 
		size_Irgb, ib, 0, NULL, NULL);
	logFile("clEnqueueWriteBuffer ...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
		logFile("%d :Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", ciErr1, __LINE__, __FILE__);
    }
}

void GenerateHDRDebevec::getDataFromOpenCLMemory()
{
	// Synchronous/blocking read of results, and check accumulated errors
	cl_int ciErr1;			// Error code var
	unsigned int size = sizeof(cl_float) * image->getHeight() * image->getWidth() * RGB_NUM_OF_CHANNELS;
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_hdr, CL_TRUE, 0, 
		size, image->getImage(), 0, NULL, NULL);
	unsigned int size_hdrpic = sizeof(unsigned char) * image->getWidth() * image->getHeight() * RGB_NUM_OF_CHANNELS;
	ciErr1 = clEnqueueReadBuffer(core->getCqCommandQueue(), cl_hdrpic, CL_TRUE, 0, 
		size_hdrpic, image->getPreviewImage(), 0, NULL, NULL);	
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

size_t* GenerateHDRDebevec::getSzGlobalWorkSize()
{
	return globalWorkSize;
}

size_t* GenerateHDRDebevec::getSzLocalWorkSize()
{
	return localWorkSize;
}