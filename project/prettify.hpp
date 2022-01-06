#include <string>
using namespace std;

extern const string mean_filter_id; // Names of the routines the user can invoke
extern const string gauss_filter_id;
extern const string median_filter_id;
extern const string threshold_mean_id;

unsigned char* read_image(char filename[], unsigned char *img, int *width, int *height);
void write_image(char filename[], unsigned char *img, int width, int height);
unsigned char* mean_filter(unsigned char *img, int width, int height, int radius);
unsigned char* gauss_filter(unsigned char *img, int width, int height, float sigma);
unsigned char* median_filter(unsigned char *img, int width, int height, int radius);
unsigned char* threshold_mean(unsigned char *img, int width, int height, int radius, int C);