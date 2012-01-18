#include "Radiance.h"

#include <string.h>
#include <math.h>

#include "../../utils/Consts.h"

Radiance::Radiance()
{
}

Radiance::Radiance(FILE *file)
{
	imageFile = file;
}

Radiance::~Radiance()
{
}

void Radiance::setFile(FILE *file)
{
	imageFile = file;
}

Image<float>* Radiance::readFile()
{
	Image<float> *image = new Image<float>(3);
	readRadianceHeader(image);
	readRadianceData(image);
	return image;
}

void Radiance::readRadianceHeader(Image<float> *image)
{
	printf("RGBE: reading header...\n");
	// read header information
	char head[255];
	float fval;
	int format = 0;
	float exposure = 1.0f;
  
	while(!feof(imageFile))
	{
		fgets(head, 200, imageFile);
		if( strcmp(head, "\n") == 0 )
            break;
		if( strcmp(head, "#?RADIANCE\n") == 0 )
		{
			// format specifier found
			format = 1;
        }
		if( strcmp(head, "#?RGBE\n") == 0 )
		{
			// format specifier found
			format = 1;
		}
		if( head[0]=='#' ) // comment found - skip
			continue;
		if( strcmp(head, "FORMAT=32-bit_rle_rgbe\n")==0 )
		{
			// header found
			continue;
		}
		if( sscanf(head, "EXPOSURE=%f", &fval) == 1 )
		{
			exposure = fval;
			printf("read exposure = %f", exposure);
		}
	}
	
	// ignore wierd exposure adjustments
	if(  exposure > 1e12 ||  exposure < 1e-12 )
		exposure = 1.0f;
	
	printf("Exposure = %f\n", exposure);
	image->setExposure(exposure);
	
	if( !format )
	{
		printf( "RGBE: no format specifier found\n" );
	}
	
	// image size
	unsigned int *width;
	unsigned int *height;
	width = new unsigned int;
	height = new unsigned int;
	char xbuf[4], ybuf[4];
	if( fgets( head, sizeof(head) / sizeof(head[0]), imageFile) == 0 || sscanf(head,"%3s %d %3s %d", ybuf, height, xbuf, width) != 4 )
	{
		printf( "RGBE: unknown image size\n" );
	}
	
	printf("RGBE: image size %dx%d\n", *width, *height);
	image->setHeight(*height);
	image->setWidth(*width);
}

inline float clampS( const float v, const float min, const float max )
{
	if( v < min ) return min;
	if( v > max ) return max;
	return v;
}

void  Radiance::rgbe2rgb(const Trgbe_pixel rgbe, float exposure, float *r, float *g, float *b)
{
	if( rgbe.e!=0 )     // a non-zero pixel
	{
		int e = rgbe.e - (int)(128 + 8);
		double f = ldexp( 1.0, e ) /* WHITE_EFFICACY / exposure*/;
		
		*r = (float)(rgbe.r * f);
		*g = (float)(rgbe.g * f);
		*b = (float)(rgbe.b * f);
		
		//bez ovoga za tmo_drago03
		/*r = clamp( *r, 0, 255 );
		*g = clamp( *g, 0, 255 );
		*b = clamp( *b, 0, 255 );*/
	}
	else
		*r = *g = *b = 0.f;
}

void Radiance::readRadianceData(Image<float> *image)
{
	// read image
	// depending on format read either rle or normal (note: only rle supported)
	Trgbe* scanline = new Trgbe[image->getWidth() * 4]; // calloc(image->getWidth() * 4, sizeof( Trgbe )); 
	Trgbe_pixel* img_rgbe = new Trgbe_pixel[image->getWidth() * image->getHeight()]; //malloc(image->getWidth() * image->getHeight() * sizeof(Trgbe_pixel));
    
	unsigned int y = 0;
	for( y = 0 ; y < image->getHeight(); y++ )
	{
		// read rle header
		printf("read line: %d \n", y);
		Trgbe header[4];
		printf("%d %d \n", sizeof(header), sizeof(Trgbe));
		fread(header, sizeof(header), 1, imageFile);
		if( header[0] != 2 || header[1] != 2 || (header[2]<<8) + header[3] != image->getWidth() )
		{
			//--- simple scanline (not rle)
			size_t rez = fread(scanline + 4, sizeof(Trgbe), 4 * image->getWidth() - 4, imageFile);
			printf("fread rez = %d\n");
			if( rez != 4 * image->getWidth() - 4 )
			{
				printf( "RGBE: not enough data to read in the simple format.\n" );
			}
			//--- yes, we've read one pixel as a header
			scanline[0]=header[0];
			scanline[1]=header[1];
			scanline[2]=header[2];
			scanline[3]=header[3];
			
			//--- write scanline to the image
			unsigned int x = 0;
			for( x = 0 ; x < image->getWidth(); x++ )
			{
				Trgbe_pixel rgbe;
				rgbe.r = scanline[4 * x + 0];
				rgbe.g = scanline[4 * x + 1];
				rgbe.b = scanline[4 * x + 2];
				rgbe.e = scanline[4 * x + 3];

				img_rgbe[y * image->getWidth() + x] = rgbe;
			}
		}
		else
		{
			//--- rle scanline
			//--- each channel is encoded separately
			int channel = 0;
			for( channel = 0 ; channel < RGBE_NUM_OF_CHANNELS; channel++ ) 
			{
				RLERead(scanline+image->getWidth() * channel, image->getWidth());
			}
			//--- write scanline to the image
			
			unsigned int x = 0;
			for( x = 0 ; x < image->getWidth(); x++ )
			{
				Trgbe_pixel rgbe;
				rgbe.r = scanline[x + image->getWidth() * 0];
				rgbe.g = scanline[x + image->getWidth() * 1];
				rgbe.b = scanline[x + image->getWidth() * 2];
				rgbe.e = scanline[x + image->getWidth() * 3];
				
				img_rgbe[y * image->getWidth() + x] = rgbe;
				//printf("%d ", scanline[x + image->getWidth() * 3]);
				//printf("%d %d %d %d | ", scanline[x + image->getWidth() * 0], scanline[x + image->getWidth() * 1], 
				//	scanline[x + image->getWidth() * 2], scanline[x + image->getWidth() * 3]);
			}
			//printf("\n");
		}

	}
	
	//creatStopwatch();
	//int timer_id = startStopwatch();
	
	for(y = 0; y < image->getHeight(); y++)
	{
		unsigned int x;
		for(x = 0; x < image->getWidth(); x++)
		{
			float r = -134;
			float g = -134;
			float b = -134;
			rgbe2rgb(img_rgbe[y * image->getWidth() + x], image->getExposure(), &r, &g, &b);
			
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 0] = r;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 1] = g;
			image->getImage()[y * image->getWidth() * 3 + x * 3 + 2] = b;

			//printf("%f %f %f | ", r, g, b);
			
			//image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 0] = clampS(r, 0.0f, 255.0f);
			//image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 1] = clampS(g, 0.0f, 255.0f);
			//image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 2] = clampS(b, 0.0f, 255.0f);
		}
		//printf("\n");


	}
	//float time = getTime(timer_id);
	//addTime("convert rgbe to rgb: ", time);
	//destroyStopwatch();
	
	delete[] img_rgbe;
	delete[] scanline;
}

void Radiance::RLERead(Trgbe* scanline, int size) //Run-length encoding
{
	int peek = 0;
	printf("%d\n", sizeof(*scanline));
	while( peek < size )
	{
		Trgbe p[2];
		size_t rez = fread(p, sizeof(p), 1, imageFile);
		//printf("fread rez = %d\n", rez);
		//printf(" %d %d", p[0], p[1]);
		if( p[0]>128 )
		{
			// a run
			int run_len = p[0]-128;
			
			while( run_len>0 )
			{
				scanline[peek++] = p[1];
				run_len--;
			}
		}
		else
		{
			// a non-run
			scanline[peek++] = p[1];
			
			int nonrun_len = p[0]-1;
			if( nonrun_len>0 )
			{
				size_t rez1 = fread(scanline+peek, sizeof(*scanline), nonrun_len, imageFile);
				//printf("fread rez1 = %d\n", rez1);
				peek += nonrun_len;
			}
		}
	}
	printf("peek %d, size %d \n", peek, size);
	if( peek!=size )
	{
		printf( "RGBE: difference in size while reading RLE scanline\n");
	}
}

int Radiance::RLEWrite(Trgbe* scanline, int size)
{
	Trgbe* scanend = scanline + size;
	while(scanline < scanend)
	{
		int run_start=0;
		int peek=0;
		int run_len=0;
		while( run_len<=4 && peek<128 && scanline+peek<scanend)
		{
			run_start=peek;
			run_len=0;
			while(run_len < 127 && run_start + run_len < 128 && scanline + peek < scanend && scanline[run_start] == scanline[peek])
			{
				peek++;
				run_len++;
			}
		}

		if( run_len>4 )
		{
			// write a non run: scanline[0] to scanline[run_start]
			if( run_start>0 )
			{
				Trgbe* buf = new Trgbe[run_start + 1];//malloc(sizeof(Trgbe) * (run_start + 1));//new Trgbe[run_start + 1];
				buf[0] = run_start;
				int i;
				for(i=0; i<run_start; i++)
					buf[i + 1] = scanline[i];
				fwrite(buf, sizeof(Trgbe), run_start + 1, imageFile);
				delete[] buf;//free(buf); //delete[] buf;
			}
			
			// write a run: scanline[run_start], run_len
			Trgbe buf[2];
			buf[0] = 128 + run_len;
			buf[1] = scanline[run_start];
			fwrite(buf, sizeof(*buf), 2, imageFile);
		}
		else
		{
			// write a non run: scanline[0] to scanline[peek]
			Trgbe* buf = new Trgbe[peek + 1]; //malloc(sizeof(Trgbe) * (peek + 1)); //new Trgbe[peek + 1];
			buf[0] = peek;
			int i;
			for(i = 0; i<peek; i++)
			{
				buf[i + 1] = scanline[i];
			}
			fwrite(buf, sizeof(Trgbe), peek + 1, imageFile);
			delete[] buf;//free(buf); //delete[] buf;
		}
		scanline += peek;
	}
  
	if( scanline!=scanend )
	{
		printf("RGBE: difference in size while writing RLE scanline\n");
		return -1;
	}
	
	return 0;
}

void  Radiance::rgb2rgbe( float r, float g, float b, Trgbe_pixel *rgbe)
{
	r /= WHITE_EFFICACY;
	g /= WHITE_EFFICACY;
	b /= WHITE_EFFICACY;
	
	double v = r;   // max rgb value
	if(v < g)
		v = g;
	if(v < b)
		v = b;
	
	if(v < 1e-32)
	{
		rgbe->r = rgbe->g = rgbe->b = rgbe->e = 0;
	}
	else
	{
		int e;  // exponent
  
		v = frexp(v, &e) * 256.0 / v;
		rgbe->r = (Trgbe)(v * r);
		rgbe->g = (Trgbe)(v * g);
		rgbe->b = (Trgbe)(v * b);
		rgbe->e = (Trgbe)(e + 128);
	}
}

void Radiance::writeFile(Image<float> *image)
{
	// header information
	fprintf(imageFile, "#?RADIANCE\n");  // file format specifier
	fprintf(imageFile, "# PFStools writer to Radiance RGBE format\n");
	fprintf(imageFile, "EXPOSURE=%f\n", image->getExposure());
    //    if( exposure_isset )
    //      fprintf(file, "EXPOSURE=%f\n", exposure);
    //    if( gamma_isset )
    //      fprintf(file, "GAMMA=%f\n", gamma);
	
	fprintf(imageFile, "FORMAT=32-bit_rle_rgbe\n");
	fprintf(imageFile, "\n");
	
	// image size
	fprintf(imageFile, "-Y %d +X %d\n", image->getHeight(), image->getWidth());
  
    // image run length encoded
	Trgbe* scanlineR = new Trgbe[image->getWidth() * image->getHeight()];//malloc(sizeof(Trgbe) * width * height); //new Trgbe[width];
	Trgbe* scanlineG = new Trgbe[image->getWidth() * image->getHeight()];//malloc(sizeof(Trgbe) * width * height); //new Trgbe[width];
	Trgbe* scanlineB = new Trgbe[image->getWidth() * image->getHeight()];//malloc(sizeof(Trgbe) * width * height); //new Trgbe[width];
	Trgbe* scanlineE = new Trgbe[image->getWidth() * image->getHeight()];//malloc(sizeof(Trgbe) * width * height); //new Trgbe[width];
	
	//creatStopwatch();
	//int timer_id = startStopwatch();
	unsigned int x,y;
	for(y = 0; y < image->getHeight(); y++)
	{
		for(x = 0; x < image->getWidth(); x++)
		{
			Trgbe_pixel p;
			rgb2rgbe(image->getImage()[y * image->getWidth() * 3 + x * 3 + 0], 
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 1], 
				image->getImage()[y * image->getWidth() * 3 + x * 3 + 2], &p);
			scanlineR[y * image->getWidth() + x] = p.r;
			scanlineG[y * image->getWidth() + x] = p.g;
			scanlineB[y * image->getWidth() + x] = p.b;
			scanlineE[y * image->getWidth() + x] = p.e;

			printf("%d %d %d %d |", p.r, p.g, p.b, p.e);
		}
		printf("\n");
	}
	
	//float time = getTime(timer_id);
	//addTime("convert rgb to rgbe: ", time);
	//destroyStopwatch();
	
	for(y=0; y < image->getHeight(); y++)
	{
		// write rle header
		unsigned char header[4];
		header[0] = 2;
		header[1] = 2;
		header[2] = image->getWidth() >> 8;;
		header[3] = image->getWidth() & 0xFF;
		fwrite(header, sizeof(header), 1, imageFile);
		
		RLEWrite(&(scanlineR[y * image->getWidth()]), image->getWidth());
		RLEWrite(&(scanlineG[y * image->getWidth()]), image->getWidth());
		RLEWrite(&(scanlineB[y * image->getWidth()]), image->getWidth());
		RLEWrite(&(scanlineE[y * image->getWidth()]), image->getWidth());
	}
	
	delete[] scanlineR; //free(scanlineR); //delete[] scanlineR;
	delete[] scanlineG; //free(scanlineG); //delete[] scanlineG;
	delete[] scanlineB; //free(scanlineB); //delete[] scanlineB;
	delete[] scanlineE; //free(scanlineE); //delete[] scanlineE;
}

