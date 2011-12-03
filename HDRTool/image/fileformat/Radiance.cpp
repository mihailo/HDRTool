#include "Radiance.h"

#include <string.h>
#include <math.h>

Radiance::Radiance(FILE *file)
{
	imageFile = file;
}

Radiance::~Radiance()
{
}

Image* Radiance::readFile()
{
	Image *image = new Image();
	readRadianceHeader(image);
	readRadianceData(image);
	return image;
}

void Radiance::readRadianceHeader(Image *image)
{
    printf("RGBE: reading header...\n");
    // read header information
    char head[255];
    float fval;
    int format = 0;
    float exposure = 1.0f;
  
    while( !feof(imageFile) )
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
			image->setExposure(exposure);
        }
      }
  
      // ignore wierd exposure adjustments
      if(  exposure > 1e12 ||  exposure < 1e-12 )
          exposure = 1.0f;
  
      printf("Exposure = %f\n", exposure);
  
      if( !format )
      {
          printf( "RGBE: no format specifier found\n" );
      }
  
      // image size
	  unsigned int width, height;
      char xbuf[4], ybuf[4];
      if( fgets( head, sizeof(head) / sizeof(head[0]), imageFile) == 0 || sscanf(head,"%3s %d %3s %d", ybuf, height, xbuf, width) != 4 )
      {
          printf( "RGBE: unknown image size\n" );
      }
  
      printf("RGBE: image size %dx%d\n", width, height);
	  image->setHeight(height);
	  image->setWidth(width);
}

inline float clampS( const float v, const float min, const float max )
{
    if( v < min ) return min;
    if( v > max ) return max;
    return v;
}

inline void rgbe2rgb(const Trgbe_pixel rgbe, float exposure, float *r, float *g, float *b)
{
    if( rgbe.e!=0 )     // a non-zero pixel
    {
        int e = rgbe.e - (int)(128 + 8);
        double f = ldexp( 1.0, e ) * WHITE_EFFICACY / exposure;
  
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

void Radiance::readRadianceData(Image *image)
{
	// read image
    // depending on format read either rle or normal (note: only rle supported)
	Trgbe* scanline = new Trgbe[image->getWidth() * 4]; // calloc(image->getWidth() * 4, sizeof( Trgbe )); 
    Trgbe_pixel* pic = new Trgbe_pixel[image->getWidth() * image->getHeight()]; //malloc(image->getWidth() * image->getHeight() * sizeof(Trgbe_pixel));
    
	int y = 0;
    for( y = 0 ; y < image->getHeight() ; y++ )
    {
        // read rle header
        Trgbe header[4];
        fread(header, sizeof(header), 1, imageFile);
        if( header[0] != 2 || header[1] != 2 || (header[2]<<8) + header[3] != image->getWidth() )
        {
            //--- simple scanline (not rle)
            size_t rez = fread(scanline + 4, sizeof(Trgbe), 4 * image->getWidth() - 4, imageFile);
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
            int x = 0;
            for( x = 0 ; x < image->getWidth(); x++ )
            {
                pic[y * image->getWidth() + x].r = scanline[4 * x + 0];
                pic[y * image->getWidth() + x].g = scanline[4 * x + 1];
                pic[y * image->getWidth() + x].b = scanline[4 * x + 2];
                pic[y * image->getWidth() + x].e = scanline[4 * x + 3];
            }
        }
        else
        {
            //--- rle scanline
            //--- each channel is encoded separately
            int channel = 0;
            for( channel = 0 ; channel < 4 ; channel++ )
                RLERead(scanline+image->getWidth() * channel, image->getWidth());
  
            //--- write scanline to the image
  
            int x = 0;
            for( x = 0 ; x < image->getWidth(); x++ )
            {
                Trgbe_pixel rgbe;
                pic[y * image->getWidth() + x].r = scanline[x + image->getWidth() * 0];
                pic[y * image->getWidth() + x].g = scanline[x + image->getWidth() * 1];
                pic[y * image->getWidth() + x].b = scanline[x + image->getWidth() * 2];
                pic[y * image->getWidth() + x].e = scanline[x + image->getWidth() * 3];
            }
        }
    }
  
  
    //creatStopwatch();
    //int timer_id = startStopwatch();
  
	for(y = 0; y < image->getHeight(); y++)
    {
        int x;
        for(x = 0; x < image->getWidth(); x++)
        {
            float r,g,b;
			rgbe2rgb(pic[y * image->getWidth() + x], image->getExposure(), &r, &g, &b);
  
			image->getHDR()[y * image->getWidth() * 3 + x * 3 + 0] = r;
            image->getHDR()[y * image->getWidth() * 3 + x * 3 + 1] = g;
            image->getHDR()[y * image->getWidth() * 3 + x * 3 + 2] = b;
  
			image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 0] = clampS(r, 0.0f, 255.0f);
            image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 1] = clampS(g, 0.0f, 255.0f);
            image->getPreviewImage()[y * image->getWidth() * 3 + x * 3 + 2] = clampS(b, 0.0f, 255.0f);
        }
    }
    //float time = getTime(timer_id);
    //addTime("convert rgbe to rgb: ", time);
    //destroyStopwatch();
  
  
    delete pic;
    delete scanline;
}

void Radiance::RLERead(Trgbe* scanline, int size) //Run-length encoding
{
    int peek = 0;
    while( peek < size )
    {
        Trgbe p[2];
        fread(p, sizeof(p), 1, imageFile);
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
                fread(scanline+peek, sizeof(*scanline), nonrun_len, imageFile);
                peek += nonrun_len;
            }
        }
    }
    if( peek!=size )
    {
        printf( "RGBE: difference in size while reading RLE scanline\n");
    }
  
}
