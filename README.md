# Profiling and Benchmarking a Java application

This repository contains my coursework submission for the System Performance Engineering course. The coursework involved profiling a Java data processing application (analysing Covid-19 case numbers to identify per-day hotspots), and creating a microbenchmark for the most expensive operation. Then reimplementing the benchmark in C++, and contrast its performance.

### Key Tasks

- **Task 1:** Profile the Java application to identify performance-critical code sections.
- **Task 2:** Extract a meaningful microbenchmark based on the most expensive part of the application.
- **Task 3:** Contrast the performance of the Java implementation with a C++ reimplementation.

## Results and Analysis

- A breakdown of the runtime of the Java application is provided in `Breakdown.txt` (Task 1).
- The hypothesis and findings of microbenchmarking are documented in `Report.txt` (Task 2 + 3).


## Repository Structure

- `TargetApplication/`: Contains the Java application using the TableSaw library for data processing.
- `JavaBenchmark/`: Contains the Java microbenchmark for most expensive part of the aggregation process implemented using JMH. 
- `CppBenchmark/`: Contains the C++ benchmark implementation using the Google Benchmark framework.

## Getting Started

### Java Application

To build the application

```bash
cd TargetApplication
mvn compile
```

To then run the Java application:

```bash
mvn exec:java -Dexec.mainClass=uk.ac.ic.doc.spe.covidsaw.App
```

### Java Benchmark

To execute the Java microbenchmark:

```bash
cd JavaBenchmark
mvn install && java -jar target/benchmarks.jar -w 1 -wi 1 -r 1 -f 1 -i 5 -bm avgt -tu ns
```

### C++ Benchmark

To build and run the C++ benchmark:

```bash
cd CppBenchmark/build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./Benchmarks
```
## Contributions

This project was completed individually as part of the coursework requirements for the System Performance Engineering course.

