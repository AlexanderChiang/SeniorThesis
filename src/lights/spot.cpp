
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
 * - I required a laser light source in order to test dispersion so 
 *   this spotlight has been modified into a laser.
 * - Laser has fixed cross-sectional area of .05 units
 * - Changed both Sample_L() functions to generate near-parallel rays
	 and return the new PDF.
 * - Falloff() no longer used
 */


// lights/spot.cpp*
#include "stdafx.h"
#include "lights/spot.h"
#include "paramset.h"
#include "montecarlo.h"

// SpotLight Method Definitions
SpotLight::SpotLight(const Transform &light2world,
                     const Spectrum &intensity, float width, float fall)
    : Light(light2world) {
    lightPos = LightToWorld(Point(0,0,0));
    Intensity = intensity;
    cosTotalWidth = cosf(Radians(width));
    cosFalloffStart = cosf(Radians(fall));
}


Spectrum SpotLight::Sample_L(const Point &p, float pEpsilon,
        const LightSample &ls, float time, Vector *wi,
        float *pdf, VisibilityTester *visibility) const {
	Point lLocal;
	lLocal.z = 0;
	UniformSampleDisk(ls.uPos[0], ls.uPos[1], &lLocal.x, &lLocal.y);
	lLocal *= .05f;
	Point pLocal = WorldToLight(p);
	Vector temp = Normalize(lLocal - pLocal);
	if (abs(temp.x) < 0.01 && abs(temp.y) < 0.01) {
		*wi = LightToWorld(temp);
		*pdf = INV_PI * INV_PI * 20;
		visibility->SetSegment(p, pEpsilon, LightToWorld(lLocal), 0., time);
		return Intensity;
	}
	else {
		return 0.f;
	}
}


float SpotLight::Falloff(const Vector &w) const {
    Vector wl = Normalize(WorldToLight(w));
    float costheta = wl.z;
    if (costheta < cosTotalWidth)     return 0.;
    if (costheta > cosFalloffStart)   return 1.;
    // Compute falloff inside spotlight cone
    float delta = (costheta - cosTotalWidth) /
                  (cosFalloffStart - cosTotalWidth);
    return delta*delta*delta*delta;
}


Spectrum SpotLight::Power(const Scene *) const {
    return Intensity * 2.f * M_PI *
           (1.f - .5f * (cosFalloffStart + cosTotalWidth));
}


SpotLight *CreateSpotLight(const Transform &l2w, const ParamSet &paramSet) {
    Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
    Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
    float coneangle = paramSet.FindOneFloat("coneangle", 30.);
    float conedelta = paramSet.FindOneFloat("conedeltaangle", 5.);
    // Compute spotlight world to light transformation
    Point from = paramSet.FindOnePoint("from", Point(0,0,0));
    Point to = paramSet.FindOnePoint("to", Point(0,0,1));
    Vector dir = Normalize(to - from);
    Vector du, dv;
    CoordinateSystem(dir, &du, &dv);
    Transform dirToZ =
        Transform(Matrix4x4( du.x,  du.y,  du.z, 0.,
                             dv.x,  dv.y,  dv.z, 0.,
                            dir.x, dir.y, dir.z, 0.,
                                0,     0,     0, 1.));
    Transform light2world = l2w *
        Translate(Vector(from.x, from.y, from.z)) * Inverse(dirToZ);
    return new SpotLight(light2world, I * sc, coneangle,
        coneangle-conedelta);
}


float SpotLight::Pdf(const Point &, const Vector &) const {
    return 0.;
}


Spectrum SpotLight::Sample_L(const Scene *scene, const LightSample &ls,
        float u1, float u2, float time, Ray *ray, Normal *Ns,
        float *pdf) const {
	Point v;
	v.z = 0;
	UniformSampleDisk(ls.uPos[0], ls.uPos[1], &v.x, &v.y);
	v *= .05f;
    *ray = Ray(LightToWorld(v), LightToWorld(Vector(0, 0, 1)), 0.f, INFINITY, time);
    *Ns = (Normal)ray->d;
    *pdf = INV_PI * INV_PI * 20;
    return Intensity;
}


