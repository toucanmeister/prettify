#include <omp.h>
#include <atomic>
#include <iostream>

using namespace std;

bool is_solution(int number) {
  for (volatile int i = 10000000; i--;) {}
  return number > 133 && number < 140;
}

int main() {
  constexpr int biggest_possible_number = 10000;
  atomic<int> solution(INT32_MAX);
  const double start = omp_get_wtime();

#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < biggest_possible_number; ++i) {
    if (solution.load() < i) // we've already found a smaller solution
      continue;
    if (is_solution(i)) {
      int previous = solution.load();
      while (previous > i && !solution.compare_exchange_weak(previous, i)) {}
    }
  }

  if (solution.load() == INT32_MAX) {
    cout << "Couldn't find a solution." << endl;
  } else {
    cout << "The solution is: " << solution.load() << endl;
  }
  cout << omp_get_wtime() - start << " seconds" << endl;
}
