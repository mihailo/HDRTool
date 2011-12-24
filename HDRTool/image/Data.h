#ifndef IOFILE_H_
#define IOFILE_H_

  

typedef unsigned char Trgbe;
struct
{
    /// @name RGB values and their exponent
    Trgbe r;
    Trgbe g;
    Trgbe b;
    Trgbe e;
} typedef Trgbe_pixel;
  
struct
{
    /// @name RGB values
    Trgbe r;
    Trgbe g;
    Trgbe b;
} typedef Trgb_pixel;

typedef unsigned char guchar;

#endif /* IOFILE_H_ */