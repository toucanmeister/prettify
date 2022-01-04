#include <iostream>
#include <string>
#include <cstring>

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
                 << "Used to remove image background, especially shadows." << endl
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
        cerr << "Error: could not read " << in_filename << endl;
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
