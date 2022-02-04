# Algorithm_Engineering

Repository for exam assignment answers, project and paper for the course Algorithm Engineering in WiSe 2021/22.

#### Answers
Contains the answers to the weekly exam assignments.

#### Project
Contains the course project. See **prettify** below for more details.

#### Paper
Contains the paper corresponding to the course project.


# prettify
Prettify is a simple command-line based scanned document enhancer.

## Features
Prettify works with **P3** (ASCII-encoded) portable pix map (**.ppm**) images. To convert to and from this format, I recommend IrfanView or `convert` on Linux.

Prettify brings 6 routines to edit images you give it:
- **Mean Filter**: Removes gausssian noise, but blurrs some edges with high radii
- **Gauss Filter**: Does the same thing as the mean filter but does not create high-frequency artifacts; is a bit slower
- **Median Filter**: Removes noise, especially salt-and-pepper noise, without blurring as much as mean or gauss filter
- **Global Threshold**: Removes backgrounds in images with clear brightness separation between fore- and background
- **Adaptive Mean Threshold**: Removes backgrounds and shadows in **text-only** images
- **Adaptive Gaussian Threshold**: Removes backgrounds and shadows in **text-only** images, sometimes leaves fewer speckles than adaptive mean threshold

A standard call to improve a scanned document image `img.ppm` would be:
```
./prettify img.ppm img_out.ppm median threshold_mean 4 10 gauss
``` 

In this call prettify:
- reads `img.ppm`
- applies a median filter with default radius
- applies an adaptive mean threshold with radius 4 and offset 10
- applies a gauss filter with default radius
- writes the finished image to `img_out.ppm`


## Usage
For more information on how to use prettify, run `prettify -h` or `prettify --help`.

## Installation
Prettify is written in C++ and thus needs to be compiled on your computer. This also means you will need a C++ compiler, CMake and the OpenMP library installed on your PC.
After downloading the source files, use a command line to go into the folder `project`.

Run the command `cmake -B build`.

Run the command `cmake --build build`.

The usable executable will be at `build/prettify`.

> Tip: To check that the program works correctly and I did not make any mistakes writing it, you can test it:
> Go into the folder `build` and run `ctest`.