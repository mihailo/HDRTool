//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
//#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable

__kernel void rgb2bw(__global const unsigned char *image, 
					   __global unsigned char *image_bw,
					   __global long *hist,
					   int height, int width)
{
	int y = get_global_id(0);
	int x = get_global_id(1);

    if (y >= height || x >= width)
    {   
        return; 
    }

	unsigned char v = (image[(y * width + x) * 3 + 0] * 54 
				+ image[(y * width + x) * 3 + 1] * 183
				+ image[(y * width + x) * 3 + 2] * 19) / 256;
	image_bw[y * width + x] = v;

	//synch ??????
	atom_add(&hist[v], 1); //= hist[v] + 1;
}
