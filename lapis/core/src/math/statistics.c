#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to calculate mean
void calculateMean(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filePath);
        return;
    }

    char line[1024];
    int columnCount = 0, rowCount = 0;
    double sums[100] = {0};  // Assuming a maximum of 100 columns

    // Read the header line
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }

    // Read the rest of the file and calculate sums
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int columnIndex = 0;
        while (token) {
            double value;
            if (sscanf(token, "%lf", &value) == 1) {  // Check if the token is numeric
                sums[columnIndex] += value;
            }
            token = strtok(NULL, ",");
            columnIndex++;
        }
        rowCount++;
    }

    fclose(file);

    // Calculate and display the means
    printf("Mean of numeric columns:\n");
    for (int i = 0; i < columnCount; i++) {
        printf("Column %d: %.2f\n", i + 1, sums[i] / rowCount);
    }
}

int main() {
    const char *filePath = "your_file.csv";  // Replace with your CSV file path
    calculateMean(filePath);
    return 0;
}
//----------------------------    median achi tale




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to compare two double values for sorting
int compare(const void *a, const void *b) {
    double val1 = *(const double *)a;
    double val2 = *(const double *)b;
    return (val1 > val2) - (val1 < val2);
}

// Function to calculate the median
void calculateMedian(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filePath);
        return;
    }

    char line[1024];
    int columnCount = 0, rowCount = 0;
    double *values[100];    // Arrays to store values for each column (max 100 columns)
    int valueCounts[100] = {0};

    // Initialize arrays for storing column values
    for (int i = 0; i < 100; i++) {
        values[i] = malloc(1000 * sizeof(double)); // Assuming a max of 1000 rows
    }

    // Read the header line
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }

    // Read the rest of the file and store values
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int columnIndex = 0;
        while (token) {
            double value;
            if (sscanf(token, "%lf", &value) == 1) {  // Check if the token is numeric
                values[columnIndex][valueCounts[columnIndex]] = value;
                valueCounts[columnIndex]++;
            }
            token = strtok(NULL, ",");
            columnIndex++;
        }
        rowCount++;
    }

    fclose(file);

    // Calculate and display the medians
    printf("Median of numeric columns:\n");
    for (int i = 0; i < columnCount; i++) {
        if (valueCounts[i] > 0) {
            // Sort the column values
            qsort(values[i], valueCounts[i], sizeof(double), compare);

            // Calculate the median
            double median;
            if (valueCounts[i] % 2 == 0) {
                median = (values[i][valueCounts[i] / 2 - 1] + values[i][valueCounts[i] / 2]) / 2.0;
            } else {
                median = values[i][valueCounts[i] / 2];
            }
            printf("Column %d: %.2f\n", i + 1, median);
        }
    }

    // Free allocated memory
    for (int i = 0; i < 100; i++) {
        free(values[i]);
    }
}

int main() {
    const char *filePath = "your_file.csv";  // Replace with your CSV file path
    calculateMedian(filePath);
    return 0;
}

//-------------------------  mode niche

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold value-frequency pairs
typedef struct {
    double value;
    int frequency;
} Frequency;

// Function to find or add a value in the frequency array
void updateFrequency(Frequency *freq, int *size, double value) {
    for (int i = 0; i < *size; i++) {
        if (freq[i].value == value) {
            freq[i].frequency++;
            return;
        }
    }
    freq[*size].value = value;
    freq[*size].frequency = 1;
    (*size)++;
}

// Function to calculate the mode
void calculateMode(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filePath);
        return;
    }

    char line[1024];
    int columnCount = 0, rowCount = 0;
    Frequency *frequencies[100];  // Frequency arrays for each column (max 100 columns)
    int freqSizes[100] = {0};     // Sizes of frequency arrays

    // Initialize arrays for storing frequencies
    for (int i = 0; i < 100; i++) {
        frequencies[i] = malloc(1000 * sizeof(Frequency)); // Assuming max 1000 unique values per column
    }

    // Read the header line
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }

    // Read the rest of the file and calculate frequencies
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int columnIndex = 0;
        while (token) {
            double value;
            if (sscanf(token, "%lf", &value) == 1) {  // Check if the token is numeric
                updateFrequency(frequencies[columnIndex], &freqSizes[columnIndex], value);
            }
            token = strtok(NULL, ",");
            columnIndex++;
        }
        rowCount++;
    }

    fclose(file);

    
    printf("Mode of numeric columns:\n");
    for (int i = 0; i < columnCount; i++) {
        if (freqSizes[i] > 0) {
            int maxFreq = 0;
            double modeValue = 0.0;
            for (int j = 0; j < freqSizes[i]; j++) {
                if (frequencies[i][j].frequency > maxFreq) {
                    maxFreq = frequencies[i][j].frequency;
                    modeValue = frequencies[i][j].value;
                }
            }
            printf("Column %d: %.2f (Frequency: %d)\n", i + 1, modeValue, maxFreq);
        }
    }

    // Free allocated memory
    for (int i = 0; i < 100; i++) {
        free(frequencies[i]);
    }
}

int main() {
    const char *filePath = "your_file.csv"; 
    calculateMode(filePath);
    return 0;
}


//----------- stndrd  dev


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Function to calculate standard deviation
void calculateStandardDeviation(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filePath);
        return;
    }

    char line[1024];
    int columnCount = 0, rowCount = 0;
    double sums[100] = {0};         // Sum of values for each column
    double squaredSums[100] = {0}; // Sum of squared values for each column
    int valueCounts[100] = {0};    // Count of numeric values in each column

    // Read the header line
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }

    // Read the rest of the file and calculate sums
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int columnIndex = 0;
        while (token) {
            double value;
            if (sscanf(token, "%lf", &value) == 1) {  // Check if the token is numeric
                sums[columnIndex] += value;
                squaredSums[columnIndex] += value * value;
                valueCounts[columnIndex]++;
            }
            token = strtok(NULL, ",");
            columnIndex++;
        }
        rowCount++;
    }

    fclose(file);

    // Calculate and display the standard deviations
    printf("Standard Deviation of numeric columns:\n");
    for (int i = 0; i < columnCount; i++) {
        if (valueCounts[i] > 1) { // Ensure there are enough values for calculation
            double mean = sums[i] / valueCounts[i];
            double variance = (squaredSums[i] / valueCounts[i]) - (mean * mean);
            double stdDev = sqrt(variance);
            printf("Column %d: %.2f\n", i + 1, stdDev);
        } else {
            printf("Column %d: Insufficient data for calculation\n", i + 1);
        }
    }
}

int main() {
    const char *filePath = "your_file.csv";  // Replace with your CSV file path
    calculateStandardDeviation(filePath);
    return 0;
}


//----------------variance





#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to calculate variance
void calculateVariance(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filePath);
        return;
    }

    char line[1024];
    int columnCount = 0, rowCount = 0;
    double sums[100] = {0};         // Sum of values for each column
    double squaredSums[100] = {0}; // Sum of squared values for each column
    int valueCounts[100] = {0};    // Count of numeric values in each column

    // Read the header line
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }

    // Read the rest of the file and calculate sums
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int columnIndex = 0;
        while (token) {
            double value;
            if (sscanf(token, "%lf", &value) == 1) {  // Check if the token is numeric
                sums[columnIndex] += value;
                squaredSums[columnIndex] += value * value;
                valueCounts[columnIndex]++;
            }
            token = strtok(NULL, ",");
            columnIndex++;
        }
        rowCount++;
    }

    fclose(file);

    // Calculate and display the variances
    printf("Variance of numeric columns:\n");
    for (int i = 0; i < columnCount; i++) {
        if (valueCounts[i] > 1) { // Ensure there are enough values for calculation
            double mean = sums[i] / valueCounts[i];
            double variance = (squaredSums[i] / valueCounts[i]) - (mean * mean);
            printf("Column %d: %.2f\n", i + 1, variance);
        } else {
            printf("Column %d: Insufficient data for calculation\n", i + 1);
        }
    }
}

int main() {
    const char *filePath = "your_file.csv";  // Replace with your CSV file path
    calculateVariance(filePath);
    return 0;
}



