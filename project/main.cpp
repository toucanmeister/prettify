#include <iostream>
#include <string>
#include <cstring>
#include <omp.h>

#include "prettify.hpp"

using namespace std;

inline void print_usage(char *program_name) {
    cout << "Usage: " << program_name << " input_file output_file [routines]" << endl;
}

// Handles the printing of help messages, returns 1 if the program should exit
int handle_help(int argc, char *argv[]) {
    if (argc == 2 && (strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0)) {
        cout << "prettify: Simple command-line based scanned document enhancer" << endl;
        print_usage(argv[0]);
        cout << " input_file and output_file need to be P3 (ASCII-encoded) portable pix map (.ppm) files without comments." << endl;
        cout << " The specified [routines] will operate on the image and may be any (even multiple) of the following, in any order: " << endl;
        cout << "   " << mean_filter_id << " [radius]" << endl 
             << "   " << gauss_filter_id << " [radius]" << endl 
             << "   " << median_filter_id << " [radius]" << endl
             << "   " << threshold_id << " [threshold]" << endl
             << "   " << threshold_adaptive_mean_id << " [radius [C]]" << endl
             << "   " << threshold_adaptive_gauss_id << " [radius [C]]" << endl;
        cout << "Try \"" << argv[0] << " -h routine\" for information on a specific routine" << endl; 
        return 1;
    }
    if ((strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0)) {
        if (mean_filter_id.compare(argv[2]) == 0) {
            cout << "Convolutional filter, replaces each pixel with the mean computed over a square around it." << endl
                 << "Used to remove ISO-noise and speckle noise, also smoothes edges; might cause high-frequency artifacts."  << endl
                 << "Usage: " << argv[0] << " input_file output_file " << mean_filter_id << " [radius]" << endl
                 << "  radius:  Determines the size of the surrounding square in which the mean is calculated." << endl;
        } else if (gauss_filter_id.compare(argv[2]) == 0) {
            cout << "Convolutional filter, replaces each pixel with the gaussian-weighted mean computed over a square around it." << endl
                 << "Used to remove ISO-noise and speckle noise, also slightly smoothes edges." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << gauss_filter_id << " [radius]" << endl
                 << "  radius:  Determines the size of the surrounding square in which the weighted mean is calculated." << endl;
        } else if (median_filter_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter, replaces each pixel with the median computed over a square around it." << endl
                 << "Used to remove ISO-noise, speckle noise and salt-and-pepper noise, also slightly smoothes edges." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << median_filter_id << " [radius]" << endl
                 << "  radius:  Ddetermines the size of the surrounding square in which the median is calculated." << endl;
        } else if (threshold_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter that makes a pixel white if it isn't darker than a specified threshold." << endl
                 << "Used to remove image background, especially shadows. Only works when background is cleanly separable by its brightness." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << threshold_id << " [threshold]" << endl
                 << "  thresh:  Determines the threshold." << endl;
        } else if (threshold_adaptive_mean_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter that makes a pixel white if its not significantly darker than the mean of its surrounding pixels." << endl
                 << "Used to remove image background, especially shadows. Only works with pure text document images." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << threshold_adaptive_mean_id << " [radius [C]]" << endl
                 << "  radius:  Determines the size of the surrounding square in which the mean is calculated." << endl
                 << "  C:       Determines how much darker than the mean a pixel has to be." << endl;
        } else if (threshold_adaptive_gauss_id.compare(argv[2]) == 0) {
            cout << "Nonlinear filter that makes a pixel white if its not significantly darker than the gaussian-weighted mean of its surrounding pixels." << endl
                 << "Used to remove image background, especially shadows. Only works with pure text document images.\nSometimes creates less noise than an adaptive mean threshold." << endl
                 << "Usage: " << argv[0] << " input_file output_file " << threshold_adaptive_gauss_id << " [radius [C]]" << endl
                 << "  radius:  Determines the size of the surrounding square in which the weighted mean is calculated." << endl
                 << "  C:       Determines how much darker than the mean a pixel has to be." << endl;
        } else {
            cout << "Either use -h or --help without further arguments or specify exactly one of the routines: " << endl;
            cout << "   " << mean_filter_id << endl 
             << "   " << gauss_filter_id << endl 
             << "   " << median_filter_id << endl
             << "   " << threshold_id << endl
             << "   " << threshold_adaptive_mean_id << endl
             << "   " << threshold_adaptive_gauss_id << endl;
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
        cerr << "Error: could not read " << in_filename << endl;
        return 1;
    }
    auto start = omp_get_wtime();
    for (int i=3; i < argc; i++) {
        if (mean_filter_id.compare(argv[i]) == 0) {
            int radius = 1;
            if (i+1 < argc && atoi(argv[i+1])) { // If the next argument is an integer, it's for this routine
                radius = atoi(argv[i+1]);
                i++; // Next argument was the radius -> skip next iteration
            }
            cout << "Applying mean filter with radius " << radius << endl;
            img = mean_filter(img, width, height, radius);
        } else if (gauss_filter_id.compare(argv[i]) == 0) {
            int radius = 1;
            if (i+1 < argc && atoi(argv[i+1])) {
                radius = atoi(argv[i+1]);
                i++;
            }
            cout << "Applying gauss filter with radius " << radius << endl;
            img = gauss_filter(img, width, height, radius);
        } else if (median_filter_id.compare(argv[i]) == 0) {
            int radius = 1;
            if (i+1 < argc && atoi(argv[i+1])) {
                radius = atoi(argv[i+1]);
                i++;
            }
            cout << "Applying median filter with radius " << radius << endl;
            img = median_filter(img, width, height, radius);
        } else if (threshold_id.compare(argv[i]) == 0){
            int thresh = 100;
            if (i+1 < argc && atoi(argv[i+1])) {
                thresh = atoi(argv[i+1]);
                i++;
            }
            cout << "Applying global threshold with threshold " << thresh << endl;
            img = threshold(img, width, height, thresh);
        } else if (threshold_adaptive_mean_id.compare(argv[i]) == 0) {
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
            cout << "Applying adaptive mean threshold with radius " << radius << " and C " << C << endl;
            img = threshold_adaptive_mean(img, width, height, radius, C);
        } else if (threshold_adaptive_gauss_id.compare(argv[i]) == 0) {
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
            cout << "Applying adaptive gauss threshold with radius " << radius << " and C " << C << endl;
            img = threshold_adaptive_gauss(img, width, height, radius, C);
        } else {
            cout << "Could not understand the following argument: " << argv[i] << endl;
            print_usage(argv[0]);
            delete[] img;
            return 0;
        }
    }
    auto end = omp_get_wtime();
    cout << "Took " << end-start << " seconds" << endl;
    write_image(out_filename, img, width, height);
    delete[] img;
    return 0;
}
