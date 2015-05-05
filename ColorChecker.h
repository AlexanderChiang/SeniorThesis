#ifndef COLORCHECKER_H
#define COLORCHECKER_H
#include "Spectrum.h"
#include "EmbreeGeometry.h"
#include "Transform.h"
#include "BxDF.h"
#include "RNG.h"

class ColorChecker {
public:
	ColorChecker(std::string fileName);

	int width;
	int height;
	std::vector<Vec3f> pixels;
	Spectrum* image;

	Spectrum CIEX, CIEY, CIEZ;
	Spectrum IllumA, IllumC, IllumD65, IllumE, custom;
	Spectrum cornellRed, cornellGreen, cornellWhite;

	Vec3f XYZtoRGB(Vec3f rgb);
	Vec3f XYZtoCIERGB(Vec3f rgb);
	Vec3f AtoD65(Vec3f rgb);
	Vec3f CtoD65(Vec3f rgb);
	Vec3f EtoD65(Vec3f rgb);

private:
	FIBITMAP* hdrMap;

	int pitch;
	int BPP;
};

#endif
