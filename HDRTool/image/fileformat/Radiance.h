#pragma once

#include <stdio.h>

#include "../Data.h"
#include "../Image.h"

class Radiance {
private:
	FILE *imageFile;
	
	void readRadianceHeader(Image *image);
	void readRadianceData(Image *image);
	void RLERead(Trgbe* scanline, int size);

	int RLEWrite(Trgbe* scanline, int size);
public:
	Radiance();
	Radiance(FILE *file);
	~Radiance();


	Image* readFile();
	void writeFile(Image *image);

	void setFile(FILE *file);

	void rgbe2rgb(const Trgbe_pixel rgbe, float exposure, float *r, float *g, float *b);
	void rgb2rgbe( float r, float g, float b, Trgbe_pixel *rgbe);
};