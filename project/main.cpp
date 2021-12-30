#include <iostream>
#include <fstream>

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

void mean_filter(unsigned char *img, int width, int height, int radius) {
    size_t size = width*height*3;
    unsigned char *new_img = new unsigned char[size];
}

int main() {
    int width, height;
    char in_filename[] = "in.ppm";
    char out_filename[] = "out.ppm";
    unsigned char *img;
    img = read_image(in_filename, img, &width, &height);
    write_image(out_filename, img, width, height);
    delete[] img;
    return 0;
}
