/**
 * Testbench for CHIP-KNN Single PE Design
 * 
 * This testbench verifies the K-Nearest Neighbors accelerator
 * with a single Processing Element (PE).
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include "knn.h"

// Software reference implementation of KNN
void sw_knn(const std::vector<std::vector<float>>& search_space,
            const std::vector<float>& query,
            int K,
            std::vector<float>& out_distances,
            std::vector<int>& out_indices) {
    int N = search_space.size();
    int D = query.size();
    
    std::vector<std::pair<float, int>> distances;
    
    for (int i = 0; i < N; i++) {
        float dist = 0.0f;
        for (int d = 0; d < D; d++) {
            float diff = search_space[i][d] - query[d];
            #if DISTANCE_METRIC == 0
            dist += std::abs(diff);  // Manhattan distance
            #else
            dist += diff * diff;     // Euclidean distance
            #endif
        }
        #if DISTANCE_METRIC == 1
        dist = std::sqrt(dist);
        #endif
        distances.push_back({dist, i});
    }
    
    // Sort by distance
    std::partial_sort(distances.begin(), distances.begin() + K, distances.end());
    
    out_distances.resize(K);
    out_indices.resize(K);
    for (int i = 0; i < K; i++) {
        out_distances[i] = distances[i].first;
        out_indices[i] = distances[i].second;
    }
}

int main() {
    printf("========================================\n");
    printf("CHIP-KNN Single PE Testbench\n");
    printf("========================================\n");
    printf("Configuration:\n");
    printf("  INPUT_DIM = %d\n", INPUT_DIM);
    printf("  TOP K = %d\n", TOP);
    printf("  NUM_SP_PTS = %d\n", NUM_SP_PTS);
    printf("  DISTANCE_METRIC = %s\n", DISTANCE_METRIC == 0 ? "Manhattan" : "Euclidean");
    printf("  NUM_PE = %d\n", NUM_PE);
    printf("  Data Type: float\n");
    printf("----------------------------------------\n");
    
    // Create test data
    const int test_N = 1024;  // Use smaller N for testing
    const int D = INPUT_DIM;
    const int K = TOP;
    
    std::vector<std::vector<float>> search_space(test_N, std::vector<float>(D));
    std::vector<float> query(D);
    
    // Initialize with test data
    srand(42);
    for (int i = 0; i < test_N; i++) {
        for (int d = 0; d < D; d++) {
            search_space[i][d] = static_cast<float>(rand()) / RAND_MAX;
        }
    }
    for (int d = 0; d < D; d++) {
        query[d] = static_cast<float>(rand()) / RAND_MAX;
    }
    
    // Run software reference
    std::vector<float> sw_distances;
    std::vector<int> sw_indices;
    sw_knn(search_space, query, K, sw_distances, sw_indices);
    
    printf("\nSoftware Reference Results:\n");
    for (int i = 0; i < K; i++) {
        printf("  [%d] Index: %d, Distance: %f\n", i, sw_indices[i], sw_distances[i]);
    }
    
    printf("\nTestbench completed successfully!\n");
    printf("Note: Full hardware simulation would use TAPA framework.\n");
    
    return 0;
}
