#include "ColorChecker.h"

typedef struct {
	double r, g, b;
} COLOR;

COLOR GetColor(double v, double vmin, double vmax) {
	COLOR c = { 1.0, 1.0, 1.0 }; // white
	double dv;

	if (v < vmin)
		v = vmin;
	if (v > vmax)
		v = vmax;
	dv = vmax - vmin;

	if (v < (vmin + 0.25 * dv)) {
		c.r = 0;
		c.g = 4 * (v - vmin) / dv;
	}
	else if (v < (vmin + 0.5 * dv)) {
		c.r = 0;
		c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
	}
	else if (v < (vmin + 0.75 * dv)) {
		c.r = 4 * (v - vmin - 0.5 * dv) / dv;
		c.b = 0;
	}
	else {
		c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
		c.b = 0;
	}

	return(c);
}

inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
	xyz[0] = 0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2];
	xyz[1] = 0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2];
	xyz[2] = 0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2];
}

void calcColorDiffPNG() {
	float currMax = 0;
	std::vector<float> values;
	std::vector<Vec3f> deltaEs;

	FIBITMAP* hdrImage = FreeImage_Load(FIF_PNG, "copperE.png", PNG_DEFAULT);
	unsigned width = FreeImage_GetWidth(hdrImage);
	unsigned height = FreeImage_GetHeight(hdrImage);
	int bpp = FreeImage_GetBPP(hdrImage);

	FIBITMAP* hdrImage2 = FreeImage_Load(FIF_PNG, "copperERGB.png");
	unsigned width2 = FreeImage_GetWidth(hdrImage);
	unsigned height2 = FreeImage_GetHeight(hdrImage);
	int bpp2 = FreeImage_GetBPP(hdrImage);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD pixel, pixel2;
			FreeImage_GetPixelColor(hdrImage, x, y, &pixel);

			float rgb[3] = { int(pixel.rgbRed) / 255.f, int(pixel.rgbGreen) / 255.f, int(pixel.rgbBlue) / 255.f };
			float xyz[3];
			RGBToXYZ(rgb, xyz);

			float var_X = xyz[0] / 95.047;   //Observer = 2°, Illuminant = D65
			float var_Y = xyz[1] / 100.000;
			float var_Z = xyz[2] / 108.883;

			if (var_X > 0.008856)
				var_X = pow(var_X, (1 / 3));
			else
				var_X = (7.787 * var_X) + (16 / 116);

			if (var_Y > 0.008856)
				var_Y = pow(var_Y, (1 / 3));
			else
				var_Y = (7.787 * var_Y) + (16 / 116);

			if (var_Z > 0.008856)
				var_Z = pow(var_Z, (1 / 3));
			else
				var_Z = (7.787 * var_Z) + (16 / 116);

			float L = (116 * var_Y) - 16;
			float a = 500 * (var_X - var_Y);
			float b = 200 * (var_Y - var_Z);


			FreeImage_GetPixelColor(hdrImage2, x, y, &pixel2);

			float rgb2[3] = { int(pixel2.rgbRed) / 255.f, int(pixel2.rgbGreen) / 255.f, int(pixel2.rgbBlue) / 255.f };
			float xyz2[3];
			RGBToXYZ(rgb2, xyz2);

			float var_X2 = xyz2[0] / 95.047;   //Observer = 2°, Illuminant = D65
			float var_Y2 = xyz2[1] / 100.000;
			float var_Z2 = xyz2[2] / 108.883;

			if (var_X2 > 0.008856)
				var_X2 = pow(var_X2, (1 / 3));
			else
				var_X2 = (7.787 * var_X2) + (16 / 116);

			if (var_Y2 > 0.008856)
				var_Y2 = pow(var_Y2, (1 / 3));
			else
				var_Y2 = (7.787 * var_Y2) + (16 / 116);

			if (var_Z2 > 0.008856)
				var_Z2 = pow(var_Z2, (1 / 3));
			else
				var_Z2 = (7.787 * var_Z2) + (16 / 116);

			float L2 = (116 * var_Y2) - 16;
			float a2 = 500 * (var_X2 - var_Y2);
			float b2 = 200 * (var_Y2 - var_Z2);

			float deltaE = sqrt((L2 - L) * (L2 - L) + (a2 - a) * (a2 - a) + (b2 - b) * (b2 - b));

			if (deltaE < 100) {
				deltaEs.push_back(Vec3f(deltaE, deltaE, deltaE));
				values.push_back(deltaE);
				if (deltaE > currMax) currMax = deltaE;
			}
			else {
				deltaEs.push_back(Vec3f(0, 0, 0));
			}
		}
	}

	FIBITMAP* image = FreeImage_AllocateT(FIT_RGBF, width, height);
	FIRGBF* output = (FIRGBF*)FreeImage_GetBits(image);

	for (unsigned int j = 0; j < height; j++) {
		for (unsigned int i = 0; i < width; i++) {
			unsigned int line = height - 1 - j;
			Vec3f pixelColor = deltaEs[width*line + i];
			output[width*line + i].red = pixelColor.x;
			output[width*line + i].green = pixelColor.y;
			output[width*line + i].blue = pixelColor.z;
		}
	}

	FreeImage_Save(FIF_EXR, image, "comp.exr", EXR_DEFAULT);

	FIBITMAP* image2 = FreeImage_AllocateT(FIT_RGBF, width, height);
	FIRGBF* output2 = (FIRGBF*)FreeImage_GetBits(image2);

	for (unsigned int j = 0; j < height; j++) {
		for (unsigned int i = 0; i < width; i++) {
			unsigned int line = height - 1 - j;
			COLOR col = GetColor(deltaEs[width * line + i].x, 0.f, .75f);
			Vec3f pixelColor = Vec3f(col.r, col.g, col.b);

			output2[width*line + i].red = pixelColor.x;
			output2[width*line + i].green = pixelColor.y;
			output2[width*line + i].blue = pixelColor.z;
		}
	}

	FreeImage_Save(FIF_EXR, image2, "comp2.exr", EXR_DEFAULT);

	float sum = 0;
	for (int x = 0; x < values.size(); x++) {
		sum += values[x];
	}

	int wait;
	std::cout << "Avg: " << sum / values.size() << std::endl;
	std::cout << "Max: " << currMax << std::endl;
	std::cin >> wait;
}

Vec3f ColorChecker::XYZtoCIERGB(Vec3f rgb) {
	Vec3f ret;
	ret[0] = 2.3706743 * rgb[0] + -0.9000405 * rgb[1] + -0.4706338 * rgb[2];
	ret[1] = -0.5138850 * rgb[0] + 1.4253036 * rgb[1] + 0.0885814 * rgb[2];
	ret[2] = 0.0052982 * rgb[0] + -0.0146949 * rgb[1] + 1.0093968 * rgb[2];

	for (int x = 0; x < 3; x++) {
		if (ret[x] <= .0031308) {
			ret[x] = 12.92 * ret[x];
		}
		else {
			ret[x] = 1.055 * pow(ret[x], 1 / 2.4f) - .055f;
		}
	}
	return ret;
}

Vec3f ColorChecker::XYZtoRGB(Vec3f rgb) {
	Vec3f ret;
	ret[0] = 3.2404542 * rgb[0] + -1.5371385 * rgb[1] + -0.4985314 * rgb[2];
	ret[1] = -0.9692660 * rgb[0] + 1.8760108 * rgb[1] + 0.0415560 * rgb[2];
	ret[2] = 0.0556434 * rgb[0] + -0.2040259 * rgb[1] + 1.0572252 * rgb[2];

	/**
	for (int x = 0; x < 3; x++) {
	if (ret[x] <= .0031308) {
	ret[x] = 12.92 * ret[x];
	}
	else {
	ret[x] = 1.055 * pow(ret[x], 1 / 2.4f) - .055f;
	}
	}
	*/
	return ret;
}

Vec3f ColorChecker::EtoD65(Vec3f rgb) {
	Vec3f ret;
	ret[0] = 0.9531874 * rgb[0] + -0.0265906 * rgb[1] + 0.0238731 * rgb[2];
	ret[1] = -0.0382467 * rgb[0] + 1.0288406 * rgb[1] + 0.0094060 * rgb[2];
	ret[2] = 0.0026068 * rgb[0] + -0.0030332 * rgb[1] + 1.0892565 * rgb[2];
	return ret;
}

Vec3f ColorChecker::AtoD65(Vec3f rgb) {
	Vec3f ret;
	ret[0] = 0.8446965 * rgb[0] + -0.1179225 * rgb[1] + 0.3948108 * rgb[2];
	ret[1] = -0.1366303 * rgb[0] + 1.1041226 * rgb[1] + 0.1291718 * rgb[2];
	ret[2] = 0.0798489 * rgb[0] + -0.1348999 * rgb[1] + 3.1924009 * rgb[2];
	return ret;
}

Vec3f ColorChecker::CtoD65(Vec3f rgb) {
	Vec3f ret;
	ret[0] = 0.9904476 * rgb[0] + -0.0071683 * rgb[1] + -0.0116156 * rgb[2];
	ret[1] = -0.0123712 * rgb[0] + 1.0155950 * rgb[1] + -0.0029282 * rgb[2];
	ret[2] = -0.0035635 * rgb[0] + 0.0067697 * rgb[1] + 0.9181569 * rgb[2];
	return ret;
}

ColorChecker::ColorChecker(std::string fileName) {
	std::string temp = fileName + "400nm.tif";
	if (temp.find(".tif") != std::string::npos) {
		hdrMap = FreeImage_Load(FIF_TIFF, temp.c_str());
	}
	else {
		std::cout << std::endl;
		std::cerr << "Error: Unsupported format in " << fileName << std::endl;
		exit(1);
	}

	if (!hdrMap) {
		std::cout << std::endl;
		std::cerr << "Error: HDR map \"" << fileName << "\" not found/loaded!" << std::endl;
		exit(1);
	}

	width = FreeImage_GetWidth(hdrMap);
	height = FreeImage_GetHeight(hdrMap);
	pitch = FreeImage_GetPitch(hdrMap);
	BPP = FreeImage_GetBPP(hdrMap);
	std::cout << "Pitch is " << pitch << std::endl;
	std::cout << BPP << " bits per pixel." << std::endl;

	image = (Spectrum*)malloc(sizeof(Spectrum) * width * height);
	for (int x = 0; x < width * height; x++) {
		image[x] = Spectrum(0.f);
	}

	int max = 0;
	for (int y = 0; y < height; y++) {
		unsigned short *bits = (unsigned short *)FreeImage_GetScanLine(hdrMap, y);
		for (int x = 0; x < width; x++) {
			if ((int)bits[x] > max)
				max = (int)bits[x];
			image[y * width + x].getC()[0] += (int)bits[x];
		}
	}

	int bucket = 1;
	for (int wavelength = 410; wavelength <= 700; wavelength += 10) {
		std::cout << "Working on bucket: " << bucket << std::endl;
		std::string temp = fileName + std::to_string(wavelength) + "nm.tif";
		hdrMap = FreeImage_Load(FIF_TIFF, temp.c_str());

		for (int y = 0; y < height; y++) {
			unsigned short *bits = (unsigned short *)FreeImage_GetScanLine(hdrMap, y);
			for (int x = 0; x < width; x++) {
				if ((int)bits[x] > max)
					max = (int)bits[x];
				image[y * width + x].getC()[bucket] += (int)bits[x];
			}
		}
		bucket++;

		std::cout << "Printing Zero Spectrum" << std::endl;
		for (int x = 0; x < 30; x++) {
			std::cout << image[0].getC()[x] << std::endl;
		}
	}

	pixels.resize(width * height);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//image[y * width + x] = image[y * width + x] / max;
			float rgb[3];
			image[y * width + x].ToXYZ(rgb);
			//std::cout << "RGB" << rgb[0] << " " << rgb[1] << " " << rgb[2] << std::endl;
			pixels[y * width + x] = Vec3f(rgb[0], rgb[1], rgb[2]);
		}
	}
}



