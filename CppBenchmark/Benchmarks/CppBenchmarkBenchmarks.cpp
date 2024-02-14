#include "../Source/CppBenchmark.hpp"
#include <benchmark/benchmark.h>


static void BM_SplitOnAndCalcMeans(benchmark::State& state) {
  // Workaround for google benchmark not taking floats
  const std::vector<double> SKEWNESS_VALUES = {0.1, 0.3, 0.5, 0.7, 0.9};
  const int NUM_ROWS = 100000;
  const int NUM_CATEGORIES = 1000; 
  double skewness = SKEWNESS_VALUES[state.range(0)];
  std::vector<Row> data = generateData(NUM_ROWS, skewness, NUM_CATEGORIES); 
  const std::vector<std::string> SPLIT_COLUMNS = {"category"};

  for (auto _ : state) {
      std::vector<TableSlice> slices; 
      simplifiedSplitOn(data, SPLIT_COLUMNS, slices); 
      benchmark::DoNotOptimize(slices);
  }
}

// Register the benchmark with the desired skewness parameters
BENCHMARK(BM_SplitOnAndCalcMeans)
    ->Args({0}) 
    ->Args({1})
    ->Args({2})
    ->Args({3})
    ->Args({4}); 

BENCHMARK_MAIN(); 
