#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

#define NUM_THREADS 4
#define PAD 8 // padding the accessed array helps with false sharing

int main() {
    int num_steps = 100000000;
    double width = 1.0 / double(num_steps);
    volatile double sum[NUM_THREADS * PAD] = {0.0};

    double start_time = omp_get_wtime();
    
#pragma omp parallel for num_threads(NUM_THREADS)
        for (int i=0; i < num_steps; i++) {
            double x = (i + 0.5) * width;
            sum[omp_get_thread_num() * PAD] += (1.0 / (1.0 + x * x));
        }

    double endsum = 0.0;
    for (int i=0; i < NUM_THREADS; i++) {
        endsum += sum[i];
    }
    double pi = endsum * 4 * width;
    double run_time = omp_get_wtime() - start_time;

    cout << "Pi ~= " << setprecision(17) << pi << endl;
    cout << "Time: " << run_time << " seconds" << endl;
}