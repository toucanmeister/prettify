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
const string threshold_adaptive_mean_id = "threshold_mean";
const string threshold_adaptive_gauss_id = "threshold_gauss";

inline unsigned int pxl(int width, int height, int row, int column) { // Translates pixel coordinates from 2d to 1d
    return row*width*3 + column*3; // We go i rows down, and j pixels to the right, times three for rgb channels
}

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
                currentVal = (unsigned) img[pxl(width, height, i, j) + c];
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
        cerr << "Error: Radius has to be at least 1." << endl;
        return img;
    }
    size_t size = width*height*3;
    unsigned char *tmp_img = new unsigned char[size];

    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    if (j+x < width && j+x >= 0) { // Edge cases are ignored, with large radii this causes shadows
                        sum += img[pxl(width, height, i, j+x) + c];
                    }
                }
                tmp_img[pxl(width, height, i, j) + c] = (unsigned char) (sum / (radius*2+1));
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
                        sum += tmp_img[pxl(width, height, i+y, j) + c];
                    }
                }
                new_img[pxl(width, height, i, j) + c] = (unsigned char) (sum / (radius*2+1));
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
// radius:   determines the size of the surrounding square in which the weighted mean is calculated
unsigned char* gauss_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius < 1)  {
        cerr << "Error: Radius has to be at least 1." << endl;
        return img;
    }
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    float sigma = ((float) radius) / 3.0; // this gives us 99% of the mass under the gaussian
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
                        sum += img[pxl(width, height, i, j+x) + c] * kernel[x+radius];
                    }
                }
                tmp_img[pxl(width, height, i, j) + c] = (unsigned char) (sum / weight);
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
                        sum += tmp_img[pxl(width, height, i+y, j) + c] * kernel[y+radius];
                    }
                }
                new_img[pxl(width, height, i, j) + c] = (unsigned char) (sum / weight);
            }
        }
    }
    delete[] img;
    delete[] tmp_img;
    return new_img;
}


// Nonlinear filter that takes the median over a square around each pixel
// radius:  determines the size of the surrounding square in which the mean is calculated
unsigned char* median_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    if (radius < 1)  {
        cerr << "Error: Radius has to be at least 1." << endl;
        return img;
    }
    

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
                        hist[img[pxl(width, height, i+y, x) + c]]++;
                    }
                }
            }
            int median = 0;
            int pxls_below_median = 0;
            for (int j=0; j < width; j++) { // Going over the pixels in one row
                if (j-radius-1 >= 0) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+y < height && i+y >= 0) {
                            int val = img[pxl(width, height, i+y, j-radius-1) + c]; // Take the left column of the last window
                            hist[val]--; // and remove it from the histogram
                            if (val < median) {
                                pxls_below_median--;
                            }
                        }
                    }
                }
                if (j+radius < width) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+y < height && i+y >= 0) {
                            int val = img[pxl(width, height, i+y, j+radius) + c]; // Take the right column of the current window
                            hist[val]++; // and add it to the histogram
                            if (val < median) {
                                pxls_below_median++;
                            }
                        }
                    }
                }
                if (pxls_below_median > n/2)  { // Median in this window is smaller than in the last
                    for (int bin=median-1; bin > 0; bin--) { // Go through the histogram
                        pxls_below_median -= hist[bin]; // Subtracting the number of pixels of each bin
                        if (pxls_below_median <= n/2) { // Until half the pixels are below the current bin
                            median = bin; // Then that bin is the median
                            break;
                        }
                    }
                } else { // Median in this window is greater than in the last
                    for (int bin=median; bin < 256; bin++) { // Go through the histogram
                        if (pxls_below_median > n/2) { // Until half the pixels are below the current bin
                            median = bin; // Then that bin is the median
                            break;
                        }
                        pxls_below_median += hist[bin]; // Adding the number of pixels of each bin
                    }
                }
                new_img[pxl(width, height, i, j) + c] = median;
            }
        }
    }
    delete[] img;
    return new_img;
}


// Nonlinear filter that makes a pixel white if it isn't darker than a specified threshold
// thresh:  determines the threshold
unsigned char* threshold(unsigned char *img, int width, int height, int thresh) {
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
unsigned char* threshold_adaptive_mean(unsigned char *img, int width, int height, int radius=5, int C=10) {
    if (radius < 1) {
        cerr << "Error: Radius has to be at least 1" << endl;
        return img;
    }
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }

    size_t size = width*height*3;
    unsigned char *copy = new unsigned char[size];
    unsigned char *tmp = new unsigned char[size];
    memcpy(copy, img, size); // Need both img and temp later, but mean_filter deletes img, so we copy
    tmp = mean_filter(img, width, height, radius); // Compute mean for each pixel
    unsigned char *new_img = new unsigned char[size];
    
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            unsigned int pixel = pxl(width, height, i, j);
            unsigned char mean_intensity = (tmp[pixel] + tmp[pixel+1] + tmp[pixel+2]) / 3;
            unsigned char pixel_intensity = (copy[pixel] + copy[pixel+1] + copy[pixel+2]) / 3;
            if (pixel_intensity > mean_intensity-C) {
                new_img[pixel] = 255;
                new_img[pixel+1] = 255;
                new_img[pixel+2] = 255;
            } else {
                new_img[pixel] = copy[pixel];
                new_img[pixel+1] = copy[pixel+1];
                new_img[pixel+2] = copy[pixel+2];
            }
        }
    }
    delete[] copy;
    delete[] tmp;
    return new_img;
}

// Nonlinear filter that makes a pixel white if its not significantly darker than the gaussian-weighted mean of its surrounding pixels
// radius:  determines the size of the surrounding square in which the mean is calculated
// C:       determines how much darker than the mean a pixel has to be
unsigned char* threshold_adaptive_gauss(unsigned char *img, int width, int height, int radius=5, int C=10) {
    if (radius < 1)  {
        cerr << "Error: Radius has to be at least 1." << endl;
        return img;
    }
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }

    size_t size = width*height*3;
    unsigned char *copy = new unsigned char[size];
    unsigned char *tmp = new unsigned char[size];
    memcpy(copy, img, size); // Need both img and temp later, but mean_filter deletes img, so we copy
    tmp = gauss_filter(img, width, height, radius); // Compute gaussian-weighted mean for each pixel
    unsigned char *new_img = new unsigned char[size];
    
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            unsigned int pixel = pxl(width, height, i, j);
            unsigned char mean_intensity = (tmp[pixel] + tmp[pixel+1] + tmp[pixel+2]) / 3;
            unsigned char pixel_intensity = (copy[pixel] + copy[pixel+1] + copy[pixel+2]) / 3;
            if (pixel_intensity > mean_intensity-C) {
                new_img[pixel] = 255;
                new_img[pixel+1] = 255;
                new_img[pixel+2] = 255;
            } else {
                new_img[pixel] = copy[pixel];
                new_img[pixel+1] = copy[pixel+1];
                new_img[pixel+2] = copy[pixel+2];
            }
        }
    }
    delete[] copy;
    delete[] tmp;
    return new_img;
}