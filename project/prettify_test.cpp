#include <iostream>
#include <cstring>
#include "prettify.hpp"

using namespace std;

#define READ_IMG 1
#define WRITE_IMG 2

int read_image_test() {
    unsigned char *img;
    int width, height;
    char filename[] = "test/in.ppm";
    img = read_image(filename, img, &width, &height);
    if (img == nullptr) {
        return 1;
    }
    delete[] img;
    return 0;
}

int write_image_test() {
    char in_filename[] = "test/in.ppm";
    char out_filename[] = "test/out.ppm";
    unsigned char *img;
    int width, height;
    img = read_image(in_filename, img, &width, &height);
    write_image(out_filename, img, width, height);
    unsigned char *result;
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
        default:
            cerr << "Unknown test number entered." << endl;
            return 1;
    }
}