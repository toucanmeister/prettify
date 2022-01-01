#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

// reads ppm p3 image at filename, returns pointer to data and width, height
unsigned char* read_image(char filename[], unsigned char *img, int *width, int *height) {
    ifstream img_file;
    img_file.open(filename, ios::in);
    char format[3];
    img_file >> format;
    int maxVal;
    img_file >> *width >> *height >> maxVal;
    size_t size = (*width) * (*height) * 3; // times 3 for each rgb channel
    img = new unsigned char[size];
    unsigned currentVal;
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
        << 255 << endl;
    size_t size = width * height * 3;
    unsigned currentVal;
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                currentVal = (unsigned) img[i*width*3 + j*3 + c];
                img_file << currentVal << " ";
            }
        }
        img_file << endl;
    }
    img_file.close();
}

// Convolutional filter, takes the mean over a square around each pixel
unsigned char* mean_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cout << "Error: Radius too large for image." << endl;
        return img;
    }
    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];

    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+x < height && i+x >= 0 && j+x < width && j+x >= 0) { // Edge cases are ignored, with large radii this causes shadows
                            sum += img[(i+x)*width*3 + (j+y)*3 + c];
                        }
                    }
                }
                new_img[i*width*3 + j*3 + c] =  (unsigned char) (sum / ((radius*2+1) * (radius*2+1)));
            }
        }
    }
    unsigned char *tmp = img;
    img = new_img;
    delete[] tmp;
    return img;
}

// Helper function for the gauss filter
inline float gauss_2d(float mu, float sigma, float x, float y) {
    return (1.0 / (2.0*M_PI*sigma*sigma)) * exp(-(x*x + y*y)/(2.0*sigma*sigma));
}

// Convolutional filter, takes the gaussian-weighted mean over a square around each pixel
unsigned char* gauss_filter(unsigned char *img, int width, int height, float sigma=1) {
    int radius = 3*sigma; // this gives us 99% of the mass under the gaussian
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cout << "Error: Sigma too large for image." << endl;
        return img;
    }
    if (sigma < 0) {
        cout << "Error: Sigma has to be positive." << endl;
    }

    float kernel[(radius*2+1) * (radius*2+1)];
    float weight = 0;
    for (int x=-radius; x <= radius; x++) {
        for (int y=-radius; y <= radius; y++) {
            float tmp = gauss_2d(0, sigma, x, y);
            kernel[(x+radius)*(2*radius+1) + (y+radius)] = tmp;
            weight += tmp;
        }
    }
    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];
    
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                float sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+x < height && i+x >= 0 && j+y < width && j+y >= 0) {
                            sum += img[(i+x)*width*3 + (j+y)*3 + c] * kernel[(x+radius)*(2*radius+1) + (y+radius)];
                        }
                    }
                }
                new_img[i*width*3 + j*3 + c] =  (unsigned char) (sum / weight);
            }
        }
    }
    unsigned char *tmp = img;
    img = new_img;
    delete[] tmp;
    return img;
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

// Nonlinear filter that takes the median over a square around each pixel
unsigned char* median_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cout << "Error: Radius too large for image." << endl;
        return img;
    }
    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];
    int n = (2*radius+1)*(2*radius+1);
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned char arr[n] = {0};
                for (int x=-radius; x <= radius; x++) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+x < height && i+x >= 0 && j+x < width && j+x >= 0) { 
                            arr[(x+radius)*(2*radius+1) + (y+radius)] = img[(i+x)*width*3 + (j+y)*3 + c];
                        }
                    }
                }
                insertionSort(arr, n);
                new_img[i*width*3 + j*3 + c] =  arr[n/2];
            }
        }
    }
    unsigned char *tmp = img;
    img = new_img;
    delete[] tmp;
    return img;
}


inline void print_usage() {
    cout << "Usage: " << argv[0] << " input_file output_file [routines]" << endl;
}

int main(int argc, char *argv[]) {
    const mean_id = "mean";
    const gauss_id = "gauss";
    const median_id = "median";
    char *in_filename;
    char *out_filename;
    if (argc < 3) {
        print_usage();
        cout << "Use " << argv[0] << " --help or " << argv[0] << " -h for more information" << endl;
    }
    if (strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0) {
        cout << "prettify: Simple command-line-based scanned document enhancer" << endl;
        print_usage();
        cout << "input_file and output_file need to be P3 (ASCII-encoded) portable pix map (.ppm) files without comments." << endl;
        cout << "[routines] can contain any (even multiple) of the following, in any order: " << endl;
        cout << mean_id << endl << gauss_id << endl << median_id << endl;
    }


    unsigned char *img;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = median_filter(img, width, height, 2);
    write_image(out_filename, img, width, height);
    delete[] img;
    return 0;
}
