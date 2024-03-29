## Profiling Overview with VTune

- The application was profiled using VTune in hotspot mode.
- **CPU Utilization:** Four threads dominated CPU utilisation. The first 15 of the 26 seconds of runtime were occupied by threads related to compiler processes. 
- **Application Execution:** The `App.main()` thread starts thereafter, marking the application's execution phase, so filtered to focus on only this thread.
- **Exclusions:** Significant time spent on loading and parsing data from CSV was excluded from detailed analysis, leaving around 2 seconds of actual data processing time.

## Detailed Breakdown:

**Data Smoothing**
- **Time Consumed:** 170 ms (23% of overall CPU time).
- **Performance Cost:** Attributed to operations within `splitOn` from `analytic::AnalyticQueryEngine::partition` → `table::StandardTableSliceGroup::create` → `table::StandardTableSliceGroup::splitOn`, and iterative actions by `java::lang::iterable::forEach`. These are necessary for organising data into partitions for smoothing calculations, each accounting for 10.8% of CPU time.

**Aggregation (Summarisation)**
- **Time Consumed:** 156 ms (21.5% of overall CPU time).
- **Performance Cost:** The `splitOn` function again was crucial, consuming 17.0% of the time, indicative of the intensive computation required for splitting and aggregating data, utilising hashmaps and a bitmap-backed selection to optimise computation.

**Filtering**
- **Time Consumed:** 100 ms (13.5% of overall CPU time).
- **Performance Cost:** Involves evaluating conditions for each row and constructing a new table with selected rows. The operations `copyRowsToTable` (accounting for 8.4% of CPU time) and `emptyCopy` are primary factors for the computational load during this phase, due to the need to copy rows to a new table and prepare a new table structure for filtered results.

**Joining**
- **Time Consumed:** 40 ms (5.4% of overall CPU time).
- **Performance Observations:** Despite the complexity of joining operations, the time taken was relatively low, possibly due to reduced dataset sizes after prior filtering and aggregation operations. This operation involves matching rows based on key columns, with `joining::DataFrameJoiner::inner` and `joining::DataFrameJoiner::crossProduct` to merge tables.

