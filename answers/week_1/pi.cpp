#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

#define NUM_THREADS 4

int main() {
    int num_steps = 100000000;
    double width = 1.0 / double(num_steps);
    volatile double sum[NUM_THREADS] = {0.0};

    double start_time = omp_get_wtime();
    
    #pragma omp parallel num_threads(NUM_THREADS)
        {
            auto thread_id = omp_get_thread_num();
            int start = (double(thread_id) / double(NUM_THREADS)) * double(num_steps);
            int end = start + num_steps/NUM_THREADS;
            for (int i=start; i < end; i++) {
                double x = (i + 0.5) * width;
                sum[thread_id] += (1.0 / (1.0 + x * x));
            }
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