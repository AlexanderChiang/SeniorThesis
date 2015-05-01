
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/*
 * Copyright (C) 2015 Alex Chiang
 *
 * - Modified to handle full spectral rendering:
 * - Added sellmeier() function which takes a wavelength and returns the 
 *   physically correct index of refraction of diamond (or flint glass) 
 *   at the specified wavelength.
 * - Added cauchy() function which takes a wavelength and returns a 
 *   physically incorrect index of refraction which is fudged so that 
 *   there is an exceptionally large difference between reds and purples.
 * - Added the switch functionality where if wavelength is set to -1, no
 *   dispersion is used. 
 */


// materials/glass.cpp*
#include "stdafx.h"
#include "materials/glass.h"
#include "spectrum.h"
#include "reflection.h"
#include "paramset.h"
#include "texture.h"

// dispersion
float sellmeier(float wavelength) {
	// micrometers conversion
	wavelength = wavelength / 1000.f;

	// diamond
	float sq = wavelength * wavelength;
	return sqrtf((.3306 * sq) / (sq - .1750 * .1750) + (4.3356 * sq) / (sq - .1060 * .1060) + 1);

	// flint glass
	//float sq = wavelength * wavelength;
	//return sqrtf((1.345 * sq) / (sq - .009977f) + (.209 * sq) / (sq - .047)  + (.937 * sq) / (sq - 111.886f) + 1);
}

float cauchy (float wavelength) {
	// prism (fudged really high coeeficient)
	wavelength = wavelength / 1000.f;
	float sq = wavelength * wavelength;
	return 1.f + .3f / sq;
}

// GlassMaterial Method Definitions
BSDF *GlassMaterial::GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena, int wavelength) const {
    DifferentialGeometry dgs;
    if (bumpMap)
        Bump(bumpMap, dgGeom, dgShading, &dgs);
    else
        dgs = dgShading;

	float ior;
	if (wavelength == -1) 
		ior = index->Evaluate(dgs);
	else
		ior = cauchy(wavelength);

    BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgs, dgGeom.nn, ior);
    Spectrum R = Kr->Evaluate(dgs).Clamp();
    Spectrum T = Kt->Evaluate(dgs).Clamp();
    if (!R.IsBlack())
        bsdf->Add(BSDF_ALLOC(arena, SpecularReflection)(R,
            BSDF_ALLOC(arena, FresnelDielectric)(1., ior)));
    if (!T.IsBlack())
        bsdf->Add(BSDF_ALLOC(arena, SpecularTransmission)(T, 1., ior));
    return bsdf;
}


GlassMaterial *CreateGlassMaterial(const Transform &xform,
        const TextureParams &mp) {
    Reference<Texture<Spectrum> > Kr = mp.GetSpectrumTexture("Kr", Spectrum(1.f));
    Reference<Texture<Spectrum> > Kt = mp.GetSpectrumTexture("Kt", Spectrum(1.f));
    Reference<Texture<float> > index = mp.GetFloatTexture("index", 1.5f);
    Reference<Texture<float> > bumpMap = mp.GetFloatTextureOrNull("bumpmap");
    return new GlassMaterial(Kr, Kt, index, bumpMap);
}


