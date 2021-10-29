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
        default_random_engine re; // had no performance increase from parallelization until i realised all threads were using the same random engine
        int counter_local = 0;
#pragma omp for
        for (int i = 0; i < n; i++) {
            auto x = zero_to_one(re);
            auto y = zero_to_one(re);
            if (x * x + y * y <= 1.0) {
                ++counter_local;
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