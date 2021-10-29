#include <iostream>
#include <omp.h>
#include <vector>

using namespace std;

struct image;

image *denoise_image(image *in, int scale) {
  image *out = nullptr;
  const int loop_until = 20000 * scale;
  for (volatile int i = 0; i < loop_until; ++i) {}
  return out;
}

int main() {
  int amount_images = 64;
  vector<image *> images(amount_images, nullptr);
  vector<image *> denoised_images(amount_images);
  double start = omp_get_wtime();
  
#pragma omp parallel for num_threads(4) schedule(dynamic)
  for (int i = amount_images - 1; i > -1; --i) {
    denoised_images[i] = denoise_image(images[i], i * i);
  }
  
  cout << omp_get_wtime() - start << " seconds" << endl;
}
