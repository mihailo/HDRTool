#include "Responses.h"

#include <math.h>


#define MIN_WEIGHT 1e-3


void Responses::exposure_weights_icip06(float* w, int M, int Mmin, int Mmax)
{
	int mi = 0;
	for(mi = 0; mi < M; mi++)
		if(mi < Mmin || mi > Mmax)
			w[mi] = 0.0f;
		else
			w[mi]=1.0f-pow(((2.0f * (float)(mi - Mmin)/(float)(Mmax - Mmin)) - 1.0f), 12.0f);
}

void Responses::weightsGauss(float* w, int M, int Mmin, int Mmax, float sigma)
{
	float mid = Mmin + (Mmax - Mmin) / 2.0f - 0.5f;
	float mid2 = (mid - Mmin) * (mid - Mmin);
	int mi;
	for(mi = 0; mi < M; mi++)
		if( mi < Mmin || mi > Mmax)
			w[mi] = 0.0f;
		else
		{
			// gkrawczyk: that's not really a gaussian, but equation is
			// taken from Robertson02 paper.
			float weight = exp( -sigma * (mi - mid) * (mi - mid) / mid2);

			if( weight < MIN_WEIGHT)           // ignore very low weights
				w[mi] = 0.0f;
			else
				w[mi] = weight;
		}
}

void Responses::weights_triangle(float* w, int M/*, int Mmin, int Mmax*/)
{
	int i;
	for(i=0; i <(int)((float)M/2.0f); i++)
	{
		w[i] = i / ((float)(M) / 2.0f); //w[i]=i/ (float(M)/2.0f);
		//if (w[i]<0.06f)w[i]=0;
	}
	for(i = (int)((float)M/2.0f); i < M; i++)
	{
		w[i] = (M - 1 - i) / ((float)M/2.0f); //w[i]=(M-1-i)/(float(M)/2.0f);
		//if (w[i]<0.06f)w[i]=0;
	}
}

void Responses::responseLinear(float* I, int M)
{
	int mi;
	for(mi = 0; mi < M ; mi++)
		I[mi] = mi / (float)(M-1); // range is not important, values are normalized later
}


void Responses::responseGamma(float* I, int M)
{
	float norm = M / 4.0f;

	// response curve decided empirically
	int mi;
	for(mi = 0; mi < M; mi++)
		I[mi] = powf( mi/norm, 1.7f ) + 1e-4;
}


void Responses::responseLog10(float* I, int M)
{
	float mid = 0.5f * M;
	float norm = 0.0625f * M;

	int mi;
	for(mi = 0; mi < M; mi++)
		I[mi] = powf(10.0f, (float)(mi - mid) / norm);
}
