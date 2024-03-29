# Report

The aggregation process's most computationally intensive part was identified as the `splitOn` function, which divides a dataset into smaller selections based on categorical columns.

## Hypothesis
- We hypothesised that the execution time of `splitOn` increases with the **skewness** of the data distribution across categories, aiming to mimic real-world scenarios where data may not be uniformly distributed across categories.
- **Rationale:** It was assumed that managing a few large groups would be more computationally demanding due to the increased cost of managing larger `Selection` objects, while also handling many small groups adds overhead in managing a larger number of keys.

### Mathematical Representation
- Let \(S\) represent skewness, and \(T\) the execution time, influenced by both the number of rows \(N\) and skewness \(S\), modeled as \(T \propto N \times g(S)\).
- Factors such as total row numbers and split column cardinality were kept constant to control other variables.

### Data Generation
- The data generator creates a table with a "Category" column to manipulate skewness and a "Value" column for aggregation.
- Skewness was adjusted by varying the probability distribution of category assignments, controlled via a `skewness` parameter ranging from 0.1 - 0.9, with higher values indicating greater skewness.

## Results
- Contrary to expectations, execution time decreased with increasing skewness. This outcome is likely due to the efficiency of `BitmapBackedSelection` and its underlying `RoaringBitmap` data structure which uses run-length encoding.
- This makes it more efficient when handling few very dense `Selections` and many sparse ones, as in highly skewed datasets - mitigating the expected impacts of skewness. 
- The decrease was likely due to the overall reduced number of categories in the more skewed datasets, which in turn reduced the number of keys to manage.

## C++ Implementation Insights
- The benchmark was reimplemented in C++, simplifying table and selection handling and introducing a run-length encoded bitmap. 
- Due to having to use the higher level `summarize().by()` function in Java, the aggregation and mean calculation was also added.

**Key Findings:**
- The C++ version showed a consistent decrease in execution time with increased skewness, mirroring Java benchmark results.
- However the decrease was less pronounced than in Java (decreased 12% compared to Java's 25%), likely due to the simplified bitmap implementation being less optimised that the `RoaringBitmap`, the C++ implementation was also substantially faster across all skewness levels.
- **Performance:** Although direct comparison is difficult due to the random data generation, benchmark results showed that the C++ implementation was about 95% faster than the Java version at a skewness level of 0.1 and maintained approximately 92% faster performance at a skewness level of 0.9.
