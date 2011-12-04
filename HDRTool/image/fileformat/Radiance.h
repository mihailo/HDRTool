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
	Radiance(FILE *file);
	~Radiance();


	Image* readFile();
	void writeFile(Image *image);
};