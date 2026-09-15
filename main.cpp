#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <future>

namespace mtt {
  class Clicker {
  public:
    Clicker():
        start_(std::chrono::high_resolution_clock::now())
    {}
    double millisec() const {
      using std::chrono::high_resolution_clock;
      using std::chrono::duration_cast;
      using std::chrono::milliseconds;
      auto t = high_resolution_clock::now();
      return duration_cast< milliseconds >(t - start_).count();
    }

  private:
    std::chrono::time_point< std::chrono::steady_clock > start_;
  };
}

using data_t = std::vector< unsigned long long >;
using value_t = data_t::value_type;

int main(int argc, char* argv[]) {
  int num_threads = 1;
  if (argc > 1) {
    num_threads = std::atoi(argv[1]);
  }

  constexpr size_t size{1'000'000'000};
  double init{0}, total{0};
  value_t sum{0};

  {
    mtt::Clicker cl;
    data_t values(size, 1);
    init = cl.millisec();

    value_t nums_count = size / num_threads;
    std::vector< std::future< value_t > > local_sums;

    for(size_t i = 0; i < num_threads; i ++) {
      value_t start = i * nums_count;
      value_t end = (i == num_threads - 1) ? size : start + nums_count;

      local_sums.push_back(std::async(std::launch::async, [start, end, &values]() -> value_t {
        value_t total = 0;
        for(value_t j = start; j < end; j++) {
          total += values[j];
        }
        return total;
      }));
    }

    for(value_t i = 0; i < local_sums.size(); i++) {
      sum += local_sums[i].get();
    }

    total = cl.millisec();
  }

  std::cout << "время подсчёта: " << (total - init) << " ms\n";
  std::cout << "время работы: " << total << " ms\n";
  std::cout << "сумма: " << sum << "\n";

  return 0;
}