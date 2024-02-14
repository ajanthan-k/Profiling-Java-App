#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm> 
#include <unordered_map>

using Run = std::pair<int, int>; 
using Selection = std::vector<Run>; 

struct Row {
    std::string category;
    double value;
};

struct TableSlice {
    std::string name;
    Selection selection;
    std::vector<Row> data; 
    double mean; 
};

class RLEBitmap {
public:
    void add(int index) { 
        if (runs.empty() || runs.back().first + runs.back().second != index) {
            runs.push_back({index, 1});
        } else {
            runs.back().second++;
        }
    }

    const Selection& getRuns() const { 
        return runs; 
    }

    // Add a size function for convenience
    int size() const {
        int count = 0;
        for (const auto& run : runs) {
            count += run.second;
        }
        return count;
    } 

private:
    Selection runs; 
};




// Pre-generate category names
std::vector<std::string> generateCategories(int count) {
    std::vector<std::string> categories(count);
    for (int i = 0; i < count; i++) {
        categories[i] = "Cat_" + std::to_string(i);
    }
    return categories;
}

std::vector<Row> generateData(int rowCount, double skewness, int numCategories) {
    std::random_device rd;
    std::mt19937 generator(rd()); 

    std::vector<std::string> categories(rowCount);
    std::vector<double> probabilities(numCategories);

    for (int i = 0; i < numCategories; i++) { 
        double bias = std::pow(skewness, 1); 
        probabilities[i] = std::pow(double(i), bias); 
    }
    std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());  

    for (int i = 0; i < rowCount; i++) {
        categories[i] = generateCategories(numCategories)[distribution(generator)];
    }

    // Fill in random values  
    std::uniform_real_distribution<double> valueDistribution(0.0, 100.0);
    std::vector<Row> data(rowCount);
    for (int i = 0; i < rowCount; i++) {
        data[i] = {categories[i], valueDistribution(generator)};
    }

    return data;
}

void simplifiedSplitOn(const std::vector<Row>& tableData, 
                       const std::vector<std::string>& splitColumns, 
                       std::vector<TableSlice>& slices) {

    // std::unordered_map: Provides similar O(1) average case lookup and insertion as 
    // Java's HashMap used within the `selectionMap`.
    std::unordered_map<std::string, RLEBitmap> selectionMap; 

    // Create selections with run-length encoding
    for (int i = 0; i < tableData.size(); i++) {
        std::string key;
        for (const auto& colName : splitColumns) {
            key += tableData[i].category + "_";
        }
        key.pop_back(); // Remove trailing '_'

        selectionMap[key].add(i); 
    }

    // Create TableSlice objects
    for (const auto& [key, selection] : selectionMap) {
        TableSlice slice;
        slice.name = key;
        slice.selection = selection.getRuns();

        // Calculate mean & populate slice data 
        double sum = 0.0;
        for (const auto& [start, length] : selection.getRuns()) {
            for (int i = start; i < start + length; i++) {
                sum += tableData[i].value;
                slice.data.push_back(tableData[i]); 
            }
        }
        slice.mean = sum / selection.size();
        slices.push_back(slice);
    }
}



// int main() {
//     const int NUM_CATEGORIES = 50;
//     const int NUM_ROWS = 10000;
//     const double SKEWNESS = 0.9;
//     const std::vector<std::string> SPLIT_COLUMNS = {"category"};

//     std::vector<Row> data = generateData(NUM_ROWS, SKEWNESS, NUM_CATEGORIES);
//     std::vector<TableSlice> slices;

//     simplifiedSplitOn(data, SPLIT_COLUMNS, slices);

//     // Output Results
//     for (const auto& slice : slices) {
//         std::cout << slice.name << "," << slice.data.size() << std::endl;
//         std::cout << std::endl;
//     }

//     return 0;
// }
