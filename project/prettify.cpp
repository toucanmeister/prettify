#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include "prettify.hpp"

using namespace std;

const string mean_filter_id = "mean"; // Names of the routines the user can invoke
const string gauss_filter_id = "gauss";
const string median_filter_id = "median";
const string threshold_id = "threshold";
const string threshold_adaptive_id = "threshold_adaptive";

// reads ppm p3 image at filename, returns pointer to data and width, height
unsigned char* read_image(char filename[], unsigned char *img, int *width, int *height) {
    ifstream img_file;
    img_file.open(filename, ios::in);
    string format;
    img_file >> format;
    if (format != "P3") {
        cerr << "Error: Input file has to be P3 (ASCII-encoded) .ppm file." << endl;
        img_file.close();
        return nullptr;
    }
    int maxVal;
    img_file >> *width >> *height >> maxVal;
    size_t size = (*width) * (*height) * 3; // times 3 for each rgb channel
    img = new unsigned char[size];
    unsigned int currentVal; // Intermediary value to read the numbers properly (char would read single chars)
    cout << "Reading image " << filename << " with width " << *width << " and height " << *height << endl;
    for (int i=0; i < size; i++) {
        img_file >> currentVal;
        img[i] = (char) currentVal; 
    }
    img_file.close();
    return img;
}

//writes data from img to ppm p3 image at filename with given width and height
void write_image(char filename[], unsigned char *img, int width, int height) {
    ofstream img_file;
    img_file.open(filename, ios::out);
    char format[3] = "P3";
    img_file << format << endl
        << width << " "
        << height << endl
        << 255 << endl; // max value for each rgb channel
    size_t size = width * height * 3;
    unsigned int currentVal;
    cout << "Writing image " << filename << endl;
    for (int i=0; i < height; i++) { // Over all rows
        for (int j=0; j < width; j++) { // Over pixels in a row
            for (int c=0; c < 3; c++) { // Over rgb channels
                currentVal = (unsigned) img[i*width*3 + j*3 + c]; // We go i rows down, and j pixels to the right, times three for rgb channels
                img_file << currentVal << " ";
            }
        }
        img_file << endl;
    }
    img_file.close();
}

// Convolutional filter, takes the mean over a square around each pixel
// radius:  determines the size of the surrounding square in which the mean is calculated
unsigned char* mean_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    if (radius < 1)  {
        cerr << "Error: Radius has to be at least 1" << endl;
        return img;
    }
    cout << "Applying mean filter with radius " << radius << endl;
    size_t size = width*height*3;
    unsigned char *tmp_img = new unsigned char[size];

    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    if (j+x < width && j+x >= 0) { // Edge cases are ignored, with large radii this causes shadows
                        sum += img[i*width*3 + (j+x)*3 + c];
                    }
                }
                tmp_img[i*width*3 + j*3 + c] = (unsigned char) (sum / (radius*2+1));
            }
        }
    }
    unsigned char *new_img = new unsigned char[size];

    for (int j=0; j < width; j++) {
        for (int i=0; i < height; i++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int y=-radius; y <= radius; y++) {
                    if (i+y < height && i+y >= 0) {
                        sum += tmp_img[(i+y)*width*3 + j*3 + c];
                    }
                }
                new_img[i*width*3 + j*3 + c] = (unsigned char) (sum / (radius*2+1));
            }
        }
    }
    delete[] img;
    delete[] tmp_img;
    return new_img;
}


// Helper function for the gauss filter
inline float gauss(float mu, float sigma, float x) {
    return (1.0 / (sqrt(2.0*M_PI)*sigma)) * exp(-(x*x)/(2.0*sigma*sigma));
}

// Convolutional filter, takes the gaussian-weighted mean over a square around each pixel
// sigma:   determines the shape of the gaussian distribution used, radius is 3*sigma
unsigned char* gauss_filter(unsigned char *img, int width, int height, float sigma=1) {
    if (sigma <= 0)  {
        cerr << "Error: Sigma has to be positive" << endl;
        return img;
    }
    int radius = (int) 3*sigma; // this gives us 99% of the mass under the gaussian
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Sigma too large for image." << endl;
        return img;
    }
    if (sigma < 0) {
        cerr << "Error: Sigma has to be positive." << endl;
        return img;
    }
    cout << "Applying gauss filter with sigma " << sigma << endl;

    float kernel[2*radius+1];
    float weight = 0;
    for (int x=-radius; x <= radius; x++) { // Initialize a 1d-kernel of normally distributed weights
        float tmp = gauss(0, sigma, x);
        kernel[x+radius] = tmp;
        weight += tmp;
    }
    size_t size = width*height*3;
    unsigned char *tmp_img = new unsigned char[size];
    
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    if (j+x < width && j+x >= 0) { // Edge cases are ignored, with large radii this causes shadows
                        sum += img[i*width*3 + (j+x)*3 + c] * kernel[x+radius];
                    }
                }
                tmp_img[i*width*3 + j*3 + c] = (unsigned char) (sum / weight);
            }
        }
    }
    unsigned char *new_img = new unsigned char[size];

    for (int j=0; j < width; j++) {
        for (int i=0; i < height; i++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int y=-radius; y <= radius; y++) {
                    if (i+y < height && i+y >= 0) {
                        sum += tmp_img[(i+y)*width*3 + j*3 + c] * kernel[y+radius];
                    }
                }
                new_img[i*width*3 + j*3 + c] = (unsigned char) (sum / weight);
            }
        }
    }
    delete[] img;
    delete[] tmp_img;
    return new_img;
}

// Helper function for median_filter
void insertionSort(unsigned char arr[], int n)
{
    int i, j;
    unsigned char key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}
/*
// Nonlinear filter that takes the median over a square around each pixel
// radius:  determines the size of the surrounding square in which the mean is calculated
unsigned char* median_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    cout << "Applying median filter with radius " << radius << endl;

    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];
    int n = (2*radius+1)*(2*radius+1);
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) { 
                unsigned char arr[n] = {0};
                for (int x=-radius; x <= radius; x++) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+x < height && i+x >= 0 && j+y < width && j+y >= 0) { 
                            arr[(x+radius)*(2*radius+1) + (y+radius)] = img[(i+x)*width*3 + (j+y)*3 + c];
                        }
                    }
                }
                insertionSort(arr, n);
                new_img[i*width*3 + j*3 + c] =  arr[n/2];
            }
        }
    }
    delete[] img;
    return new_img;
}
*/

// Nonlinear filter that takes the median over a square around each pixel
// radius:  determines the size of the surrounding square in which the mean is calculated
unsigned char* median_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    if (radius < 1)  {
        cerr << "Error: Radius has to be at least 1" << endl;
        return img;
    }
    cout << "Applying median filter with radius " << radius << endl;

    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];
    int n = (2*radius+1)*(2*radius+1);
    int hist[256];
    for (int c=0; c < 3; c++) {
        for (int i=0; i < height; i++) {
            for (int bin=0; bin < 256; bin++) { // Clear histogram
                hist[bin] = 0;
            }
            for (int y=-radius; y <= radius; y++) { // Initialize first histogram
                for (int x=0; x <= (radius-1); x++) {
                    if (i+y < height && i+y >= 0) { // Ignore edges
                        hist[img[(i+y)*width*3 + x*3 + c]]++;
                    }
                }
            }
            for (int j=0; j < width; j++) {
                for (int y=-radius; y <= radius; y++) {
                    if (i+y < height && i+y >= 0) {
                        if (j-radius >= 0) {
                            hist[img[(i+y)*width*3 + (j-radius)*3 + c]]--; // Remove left column of the last window from histogram
                        }
                        if (j+radius < width) {
                            hist[img[(i+y)*width*3 + (j+radius)*3 + c]]++; // Add right column of new window to histogram
                        }
                    }
                }
                int count = 0;
                int median;
                for (int bin=0; bin < 256; bin++) { // Go through the histogram
                    count += hist[bin]; // Adding up the number of pixels for each intensity
                    if (count >= n/2) { // Once we have half of the pixels we found the median
                        median = bin; 
                        break;
                    }
                }
                new_img[i*width*3 + j*3 + c] = median;
            }
        }
    }
    delete[] img;
    return new_img;
}


// Nonlinear filter that makes a pixel white if it isn't darker than a specified threshold
// thresh:  determines the threshold
unsigned char* threshold(unsigned char *img, int width, int height, int thresh) {
    cout << "Applying global threshold with threshold " << thresh << endl;

    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];

    for (int p=0; p < size; p+=3) { // Go over all pixels
        unsigned int sum = 0;
        for (int c=0; c < 3; c++) {
            sum += img[p+c];
        }
        unsigned char pixel_intensity = (unsigned char) (sum / 3);
        if (pixel_intensity > thresh) {
            new_img[p] = 255;
            new_img[p+1] = 255;
            new_img[p+2] = 255;
        } else {
            new_img[p] = img[p];
            new_img[p+1] = img[p+1];
            new_img[p+2] = img[p+2];
        }
    }
    delete[] img;
    return new_img;
}

// Nonlinear filter that makes a pixel white if its not significantly darker than the mean of its surrounding pixels
// radius:  determines the size of the surrounding square in which the mean is calculated
// C:       determines how much darker than the mean a pixel has to be
unsigned char* threshold_adaptive(unsigned char *img, int width, int height, int radius=5, int C=10) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    cout << "Applying adaptive mean threshold with radius " << radius << " and C " << C << endl;

    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];

    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            unsigned int sum = 0;
            for (int x=-radius; x <= radius; x++) {
                for (int y=-radius; y <= radius; y++) {
                    for (int c=0; c < 3; c++) {
                        if (i+x < height && i+x >= 0 && j+y < width && j+y >= 0) {
                            sum += img[(i+x)*width*3 + (j+y)*3 + c];
                        }
                    }
                }
            }
            unsigned char mean = (unsigned char) (sum / ((radius*2+1) * (radius*2+1) * 3));
            unsigned int pixel = i*width*3 + j*3;
            unsigned char pixel_intensity = (img[pixel] + img[pixel+1] + img[pixel+2]) / 3;
            if (pixel_intensity > mean-C) {
                new_img[pixel] = 255;
                new_img[pixel+1] = 255;
                new_img[pixel+2] = 255;
            } else {
                new_img[pixel] = img[pixel];
                new_img[pixel+1] = img[pixel+1];
                new_img[pixel+2] = img[pixel+2];
            }
        }
    }
    delete[] img;
    return new_img;
}