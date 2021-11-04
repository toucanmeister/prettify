#include <iomanip>
#include <iostream>
#include <thread>

using namespace std;

#define NUM_THREADS 8


void integrate(int thread_num, int num_steps, double width, double* sums) {
    double local_sum = 0.0;
    for (int i=thread_num; i < num_steps; i += NUM_THREADS) {
        double x = (i + 0.5) * width;
        local_sum += (1.0 / (1.0 + x * x));
    }
    sums[thread_num] = local_sum;
}


int main() {
  int num_steps = 100000000;
  double width = 1.0 / double(num_steps);

  thread threads[NUM_THREADS];
  double sums[NUM_THREADS];

  for (int i=0; i < NUM_THREADS; i++) {
    threads[i] = thread(integrate, i, num_steps, width, sums);
  }
  for (int i=0; i < NUM_THREADS; i++) {
    threads[i].join();
  }

  double sum = 0.0;
  for (int i=0; i < NUM_THREADS; i++) {
    sum += sums[i];
  }
  double pi = sum * 4 * width;

  cout << "pi with " << num_steps << " steps is " << setprecision(17) << pi << endl;
}
