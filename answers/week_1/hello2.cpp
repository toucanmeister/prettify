#include <iostream>

using namespace std;

int main() {
#pragma omp parallel num_threads(4)
    {
        cout << "Hello World!" << endl;
    }
}