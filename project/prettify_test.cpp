#include <iostream>
#include <cstring>
#include "prettify.hpp"

using namespace std;

#define READ_IMG 1
#define WRITE_IMG 2
#define MEAN_FILTER 3
#define GAUSS_FILTER 4
#define MEDIAN_FILTER 5
#define THRESHOLD 6
#define THRESHOLD_ADAPTIVE 7

int read_image_test() {
    unsigned char *img;
    int width, height;
    char filename[] = "../test/in.ppm";
    img = read_image(filename, img, &width, &height);
    if (img == nullptr) {
        return 1;
    }
    delete[] img;
    return 0;
}

int write_image_test() {
    char in_filename[] = "../test/in.ppm";
    char out_filename[] = "../test/out.ppm";
    unsigned char *img, *result;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    write_image(out_filename, img, width, height);
    result = read_image(out_filename, result, &width, &height);
    if (img == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (*(img+i) != *(result+i)) {
            delete[] img;
            delete[] result;
            return 1;
        }
    }
    delete[] img;
    delete[] result;
    return 0;
}

int mean_filter_test() {
    char in_filename[] = "../test/in.ppm";
    char check_filename[] = "../test/out_mean.ppm";
    unsigned char *img, *check;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = mean_filter(img, width, height, 1);
    if (img == nullptr) {
        return 1;
    }
    check = read_image(check_filename, check, &width, &height);
    if (check == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (abs(*(img+i) - *(check+i)) > 2) { // 2 units of wiggle-room to allow for rounding errors
            delete[] img;
            delete[] check;
            return 1;
        }
    }
    delete[] img;
    delete[] check;
    return 0;
}

int gauss_filter_test() {
    char in_filename[] = "../test/in.ppm";
    char check_filename[] = "../test/out_gauss.ppm";
    unsigned char *img, *check;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = gauss_filter(img, width, height, 1);
    if (img == nullptr) {
        return 1;
    }
    check = read_image(check_filename, check, &width, &height);
    if (check == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (abs(*(img+i) - *(check+i)) > 2) { // 2 units of wiggle-room to allow for rounding errors
            delete[] img;
            delete[] check;
            return 1;
        }
    }
    delete[] img;
    delete[] check;
    return 0;
}

int median_filter_test() {
    char in_filename[] = "../test/in.ppm";
    char check_filename[] = "../test/out_median.ppm";
    unsigned char *img, *check;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = median_filter(img, width, height, 1);
    if (img == nullptr) {
        return 1;
    }
    check = read_image(check_filename, check, &width, &height);
    if (check == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (abs(*(img+i) - *(check+i)) > 2) { // 2 units of wiggle-room to allow for rounding errors
            delete[] img;
            delete[] check;
            return 1;
        }
    }
    delete[] img;
    delete[] check;
    return 0;
}

int threshold_test() {
    char in_filename[] = "../test/in.ppm";
    char check_filename[] = "../test/out_threshold.ppm";
    unsigned char *img, *check;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = threshold(img, width, height, 100);
    if (img == nullptr) {
        return 1;
    }
    check = read_image(check_filename, check, &width, &height);
    if (check == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (abs(*(img+i) - *(check+i)) > 2) { // 2 units of wiggle-room to allow for rounding errors
            delete[] img;
            delete[] check;
            return 1;
        }
    }
    delete[] img;
    delete[] check;
    return 0;
}

int threshold_adaptive_test() {
    char in_filename[] = "../test/in.ppm";
    char check_filename[] = "../test/out_threshold_adaptive.ppm";
    unsigned char *img, *check;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    img = threshold_adaptive(img, width, height, 4, 10);
    if (img == nullptr) {
        return 1;
    }
    check = read_image(check_filename, check, &width, &height);
    if (check == nullptr) {
        return 1;
    }
    for (int i=0; i < width*height*3; i++) {
        if (abs(*(img+i) - *(check+i)) > 2) { // 2 units of wiggle-room to allow for rounding errors
            delete[] img;
            delete[] check;
            return 1;
        }
    }
    delete[] img;
    delete[] check;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0;
    }
    int testnum = atoi(argv[1]);
    switch (testnum) {
        case READ_IMG:
            return read_image_test();
            break;
        case WRITE_IMG:
            return write_image_test();
            break;
        case MEAN_FILTER:
            return mean_filter_test();
            break;
        case GAUSS_FILTER:
            return gauss_filter_test();
            break;
        case MEDIAN_FILTER:
            return median_filter_test();
            break;
        case THRESHOLD:
            return threshold_test();
            break;
        case THRESHOLD_ADAPTIVE:
            return threshold_adaptive_test();
            break;
        default:
            cerr << "Unknown test number entered." << endl;
            return 1;
    }
}
