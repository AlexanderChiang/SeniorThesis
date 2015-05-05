This is the code repository for Alex Chiang's senior thesis.

Read the write-up here: http://www.alexanderchiang.com/thesis.html

Much of this code is simply the pbrt renderer from Physically Based Rendering by Pharr and Humphreys.
The modified files are:
spectrum.h
renderer.h
samplerrenderer.h
material.h
glass.cpp
spot.cpp
path.cpp
photonmap.cpp
metal.pbrt
The pbrt portion of the project should be compile-able as is by opening the VS2010 solution.

The new files are:
ColorChecker.h
ColorChecker.cpp
These two files will require downloading of the FreeImage library and linking it into your project to compile.
