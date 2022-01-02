#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

using namespace std;

const string mean_filter_id = "mean"; // Names of the routines the user can invoke
const string gauss_filter_id = "gauss";
const string median_filter_id = "median";
const string threshold_mean_id = "threshold_mean";

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
    cout << "Writing image " << filename << endl;
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
// radius:  determines the size of the surrounding square in which the mean is calculated
unsigned char* mean_filter(unsigned char *img, int width, int height, int radius=1) {
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Radius too large for image." << endl;
        return img;
    }
    cout << "Applying mean filter with radius " << radius << endl;

    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];

    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            for (int c=0; c < 3; c++) {
                unsigned int sum = 0;
                for (int x=-radius; x <= radius; x++) {
                    for (int y=-radius; y <= radius; y++) {
                        if (i+x < height && i+x >= 0 && j+y < width && j+y >= 0) { // Edge cases are ignored, with large radii this causes shadows
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
// sigma:   determines the shape of the gaussian distribution used, radius is 3*sigma
unsigned char* gauss_filter(unsigned char *img, int width, int height, float sigma=1) {
    int radius = 3*sigma; // this gives us 99% of the mass under the gaussian
    if (radius > (width/2)-1 || radius > (height/2)-1) {
        cerr << "Error: Sigma too large for image." << endl;
        return img;
    }
    if (sigma < 0) {
        cerr << "Error: Sigma has to be positive." << endl;
        return img;
    }
    cout << "Applying gauss filter with sigma " << sigma << endl;

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
    unsigned char *tmp = img;
    img = new_img;
    delete[] tmp;
    return img;
}

// Nonlinear filter that makes a pixel white if its not significantly darker than the mean of its surrounding pixels
// radius:  determines the size of the surrounding square in which the mean is calculated
// C:       determines how much darker than the mean a pixel has to be
unsigned char* threshold_mean(unsigned char *img, int width, int height, int radius=5, int C=10) {
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
    unsigned char *tmp = img;
    img = new_img;
    delete[] tmp;
    return img;
}


inline void print_usage(char *program_name) {
    cout << "Usage: " << program_name << " input_file output_file [routines]" << endl;
}

// Handles the printing of help messages, returns 1 if the program should exit
int handle_help(int argc, char *argv[]) {
    if (argc == 2 && (strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0)) {
        cout << "prettify: Simple command-line based scanned document enhancer" << endl;
        print_usage(argv[0]);
        cout << " input_file and output_file need to be P3 (ASCII-encoded) portable pix map (.ppm) files without comments." << endl;
        cout << " [routines] can contain any (even multiple) of the following, in any order: " << endl;
        cout << "   " << mean_filter_id << " [radius]," << endl 
             << "   " << gauss_filter_id << " [sigma]," << endl 
             << "   " << median_filter_id << " [radius]," << endl
             << "   " << threshold_mean_id << " [radius [C]]" << endl;
        cout << "Try \"" << argv[0] << " -h routine\" for information on a specific routine" << endl; 
        return 1;
    }
    if ((strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0)) {
        if (mean_filter_id.compare(argv[2]) == 0) {
            cout << "Convolutional filter, replaces each pixel with the mean over a square around it." << endl
                 << "Used to remove ISO-noise and speckle noise, also smoothes edges; might cause high-frequency artifacts."  << endl
                 << "Usage: " << argv[0] << " input_file output_file " << mean_filter_id << " [radius]" << endl
                 << "  radius:  determines the size of the surrounding square in which the mean is calculated." << endl;
        } else if (gauss_filter_id.compare(argv[2]) == 0) {
            cout << "Convolutional filter, replaces each pixel with the gaussian-weighted mean over a square around it." << endl
                 << "Used to remove ISO-noise and speckle noise, also slightly smoothes edges." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << gauss_filter_id << " [sigma]" << endl
                 << "  sigma:   determines the shape of the gaussian distribution used, radius is 3*sigma" << endl;
        } else if (median_filter_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter, replaces each pixel with the median over a square around it." << endl
                 << "Used to remove ISO-noise, speckle noise and salt-and-pepper noise, also slightly smoothes edges." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << median_filter_id << " [radius]" << endl
                 << "  radius:  determines the size of the surrounding square in which the median is calculated" << endl;
        } else if (threshold_mean_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter that makes a pixel white if its not significantly darker than the mean of its surrounding pixels." << endl
                 << "Used to remove shadows from the background." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << threshold_mean_id << " [radius [C]]" << endl
                 << "  radius: determines the size of the surrounding square in which the mean is calculated" << endl
                 << "  C: determines how much darker than the mean a pixel has to be" << endl;
        } else {
            cout << "Either use -h or --help without further arguments or specify exactly one of the routines: " << endl;
            cout << "   " << mean_filter_id << endl 
             << "   " << gauss_filter_id << endl 
             << "   " << median_filter_id << endl
             << "   " << threshold_mean_id << endl;
        }
        return 1;
    }
    if (argc < 3) {
        print_usage(argv[0]);
        cout << "Try \"" << argv[0] << " --help\" or \"" << argv[0] << " -h\" for more information" << endl;
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *in_filename;
    char *out_filename;

    if (handle_help(argc, argv)) {
        return 0;
    }
    
    in_filename = argv[1];
    out_filename = argv[2];


    unsigned char *img;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    if (img == nullptr) {
        return 1;
    }

    for (int i=3; i < argc; i++) {
        if (mean_filter_id.compare(argv[i]) == 0) {
            int radius = 1;
            if (i+1 < argc && atoi(argv[i+1])) {
                radius = atoi(argv[i+1]);
                i++; // Next argument was the radius -> skip next iteration
            }
            img = mean_filter(img, width, height, radius);
        } else if (gauss_filter_id.compare(argv[i]) == 0) {
            int sigma = 1;
            if (i+1 < argc && atoi(argv[i+1])) {
                sigma = atoi(argv[i+1]);
                i++;
            }
            img = gauss_filter(img, width, height, sigma);
        } else if (median_filter_id.compare(argv[i]) == 0) {
            int radius = 1;
            if (i+1 < argc && atoi(argv[i+1])) {
                radius = atoi(argv[i+1]);
                i++;
            }
            img = median_filter(img, width, height, radius);
        } else if (threshold_mean_id.compare(argv[i]) == 0) {
            int radius = 5;
            int C = 10;
            if (i+1 < argc && atoi(argv[i+1])) {
                radius = atoi(argv[i+1]);
                i++;
                if (i+1 < argc && atoi(argv[i+1])) {
                    C = atoi(argv[i+1]);
                    i++;
                }
            }
            img = threshold_mean(img, width, height, radius, C);
        } else {
            print_usage(argv[0]);
            delete[] img;
            return 0;
        }
    }
    write_image(out_filename, img, width, height);
    delete[] img;
    return 0;
}
