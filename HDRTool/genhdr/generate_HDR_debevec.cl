float clamps( const float v, const float min, const float max )
{
  	if( v < min ) return min;
  	if( v > max ) return max;
  	return v;
}
__kernel void generate_hdri_pixels(unsigned int const width, unsigned int const height,
								   unsigned int N, unsigned int *cl_ldr_img,
									float *cl_arrayofexptime, 
									int *cl_i_lower, int *cl_i_upper, 
									int maxM, int minM, float *cl_w, 
									float *cl_Ir, float *cl_Ig, float *cl_Ib,
									float *cl_hdr, unsigned int *cl_hdrpic)
{
	int i;
	int y = get_global_id(0);
	int x = get_global_id(1);
	int j = y * width + x;
	
	if(j < width * height){
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
		for(i = 0; i < N; i++)
		{
			//pick the 3 channel values + alpha
			int mR = cl_ldr_img[i * width * height * 3 + j * 3 + 0];
			int mG = cl_ldr_img[i * width * height * 3 + j * 3 + 1];
			int mB = cl_ldr_img[i * width * height * 3 + j * 3 + 2];
			int mA = 255;// posto ga posle koristi !!!! int mA = qAlpha(* ( (QRgb*)( (listLDR->at(i) )->bits() ) + j ) );
			
			float ti = arrayofexptime_cuda[i];
			// --- anti ghosting: monotonous increase in time should result
			// in monotonous increase in intensity; make forward and
			// backward check, ignore value if condition not satisfied
			int R_lower = cl_ldr_img[i_lower_cuda[i] * width * height * 3 + j * 3 + 0];//qRed  (* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int R_upper = cl_ldr_img[i_upper_cuda[i] * width * height * 3 + j * 3 + 0];//qRed  (* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );
			int G_lower = cl_ldr_img[i_lower_cuda[i] * width * height * 3 + j * 3 + 1];//qGreen(* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int G_upper = cl_ldr_img[i_upper_cuda[i] * width * height * 3 + j * 3 + 1];//qGreen(* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );
			int B_lower = cl_ldr_img[i_lower_cuda[i] * width * height * 3 + j * 3 + 2];//qBlue (* ( (QRgb*)( (listLDR->at(i_lower[i]) )->bits() ) + j ) );
			int B_upper = cl_ldr_img[i_upper_cuda[i] * width * height * 3 + j * 3 + 3];//qBlue (* ( (QRgb*)( (listLDR->at(i_upper[i]) )->bits() ) + j ) );

			//if at least one of the color channel's values are in the bright "not-trusted zone" and we have min exposure time
			if ((mR > maxM || mG > maxM || mB > maxM) && (ti < minti))
			{
				//update the indexes_for_whiteRGB, minti
				index_for_whiteR = mR;
				index_for_whiteG = mG;
				index_for_whiteB = mB;
				minti = ti;
				//continue;
			}

			//if at least one of the color channel's values are in the dim "not-trusted zone" and we have max exposure time
			if ((mR < minM || mG < minM || mB < minM) && (ti > maxti))
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
			float w_average = mA * (cl_w[mR] + cl_w[mG] + cl_w[mB]) / (3.0f * 255.0f);
			sumR += w_average * cl_Ir[mR] / (float)ti;
			divR += w_average;
			sumG += w_average * cl_Ig[mG] / (float)ti;
			divG += w_average;
			sumB += w_average * cl_Ib[mB] / (float)ti;
			divB += w_average;
		} //END for all the exposures

		if(divR == 0.0f || divG == 0.0f || divB == 0.0f)
		{
			if (maxti > -1e6f)
			{
				sumR = cl_Ir[index_for_blackR] / (float)maxti;
				sumG = cl_Ig[index_for_blackG] / (float)maxti;
				sumB = cl_Ib[index_for_blackB] / (float)maxti;
				divR = divG = divB = 1.0f;
			}
			else
				if (minti < +1e6f)
				{
					sumR = cl_Ir[index_for_whiteR] / (float)minti;
					sumG = cl_Ig[index_for_whiteG] / (float)minti;
					sumB = cl_Ib[index_for_whiteB] / (float)minti;
					divR = divG = divB = 1.0f;
				}
		}

		if(divR != 0.0f && divG != 0.0f && divB != 0.0f)
		{
			//Rout_cuda[j] = sumR/divR;
			//Gout_cuda[j] = sumG/divG;
			//Bout_cuda[j] = sumB/divB;

			cl_hdr[j * 3 + 0] = sumR/divR;
			cl_hdr[j * 3 + 1] = sumG/divG;
			cl_hdr[j * 3 + 2] = sumB/divB;

			cl_hdrpic[j * 3 + 0] = (guchar)clamps(cl_hdr[j * 3 + 0], 0.0f, 255.0f);
			cl_hdrpic[j * 3 + 1] = (guchar)clamps(cl_hdr[j * 3 + 1], 0.0f, 255.0f);
			cl_hdrpic[j * 3 + 2] = (guchar)clamps(cl_hdr[j * 3 + 2], 0.0f, 255.0f);
		}
		else
		{
			//we shouldn't be here anyway...
			//printf("jel ulazim ovde");
			//Rout_cuda[j] = 0.0f;
			//Gout_cuda[j] = 0.0f;
			//Bout_cuda[j] = 0.0f;

			cl_hdr[j * 3 + 0] = 0.0f;
			cl_hdr[j * 3 + 1] = 0.0f;
			cl_hdr[j * 3 + 2] = 0.0f;

			cl_hdrpic[j * 3 + 0] = (unsigned int)cl_hdr[j * 3 + 0];
			cl_hdrpic[j * 3 + 1] = (unsigned int)cl_hdr[j * 3 + 1];
			cl_hdrpic[j * 3 + 2] = (unsigned int)cl_hdr[j * 3 + 2];
		}
	}
}