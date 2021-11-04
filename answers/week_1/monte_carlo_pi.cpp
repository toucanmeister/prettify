#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
    uniform_real_distribution<double> zero_to_one{0.0, 1.0};

    int n = 100000000;
    int counter = 0;
    auto start_time = omp_get_wtime();

    #pragma omp parallel num_threads(8)
    {
        int num_threads = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int counter_local = 0;
        default_random_engine re; // had no performance increase from parallelization until i realised all threads were using the same random engine
        for (int i = thread_id; i < n; i += num_threads) {
            auto x = zero_to_one(re);
            auto y = zero_to_one(re);
            if (x * x + y * y <= 1.0) {
                counter_local += 1;
            }
        }
        #pragma omp atomic 
            counter += counter_local;
    }

    auto run_time = omp_get_wtime() - start_time;
    auto pi = 4 * (double(counter) / n);
    cout << "pi: " << pi << endl;
    cout << "run_time: " << run_time << " s" << endl;
    cout << "n: " << n << endl; 
}