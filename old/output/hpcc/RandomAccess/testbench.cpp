#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

typedef unsigned long long u64Int_t;
typedef long long s64Int_t;

#define POLY 0x0000000000000007ULL
#define PERIOD 1317624576693539401LL

// Simple starts function for RandomAccess
static u64Int_t HPCC_starts(s64Int_t n) {
    if (n == 0) return 1;
    
    int i, j;
    u64Int_t m2[64];
    u64Int_t temp = 1;
    for (i = 0; i < 64; i++) {
        m2[i] = temp;
        temp = (temp << 1) ^ (((s64Int_t)temp < 0) ? POLY : 0);
        temp = (temp << 1) ^ (((s64Int_t)temp < 0) ? POLY : 0);
    }
    
    for (i = 62; i >= 0; i--) {
        if ((n >> i) & 1) break;
    }
    
    u64Int_t ran = 2;
    while (i > 0) {
        temp = 0;
        for (j = 0; j < 64; j++) {
            if ((ran >> j) & 1) temp ^= m2[j];
        }
        ran = temp;
        i--;
        if ((n >> i) & 1) {
            ran = (ran << 1) ^ (((s64Int_t)ran < 0) ? POLY : 0);
        }
    }
    return ran;
}

// RandomAccess core kernel
void RandomAccessUpdate(u64Int_t TableSize, u64Int_t *Table, u64Int_t numUpdates) {
    u64Int_t ran = 1;
    
    for (u64Int_t i = 0; i < numUpdates; i++) {
        ran = (ran << 1) ^ (((s64Int_t)ran < 0) ? POLY : 0);
        Table[ran & (TableSize - 1)] ^= ran;
    }
}

// Testbench for RandomAccess
int main() {
    const u64Int_t TableSize = 1024;  // Small test size (must be power of 2)
    const u64Int_t numUpdates = 4 * TableSize;
    
    // Allocate table
    u64Int_t *Table = (u64Int_t*)malloc(TableSize * sizeof(u64Int_t));
    if (!Table) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize table
    for (u64Int_t i = 0; i < TableSize; i++) {
        Table[i] = i;
    }
    
    // Perform RandomAccess updates
    RandomAccessUpdate(TableSize, Table, numUpdates);
    
    // Verify that table was modified (not equal to initial values)
    int modified = 0;
    for (u64Int_t i = 0; i < TableSize; i++) {
        if (Table[i] != i) {
            modified = 1;
            break;
        }
    }
    
    // Verify reproducibility: run again with same seed should give same result
    u64Int_t *Table2 = (u64Int_t*)malloc(TableSize * sizeof(u64Int_t));
    if (!Table2) {
        free(Table);
        return 1;
    }
    
    for (u64Int_t i = 0; i < TableSize; i++) {
        Table2[i] = i;
    }
    
    RandomAccessUpdate(TableSize, Table2, numUpdates);
    
    int reproducible = 1;
    for (u64Int_t i = 0; i < TableSize; i++) {
        if (Table[i] != Table2[i]) {
            reproducible = 0;
            break;
        }
    }
    
    int pass = modified && reproducible;
    printf("RandomAccess Test %s (modified=%d, reproducible=%d)\n", 
           pass ? "PASSED" : "FAILED", modified, reproducible);
    
    free(Table);
    free(Table2);
    
    return pass ? 0 : 1;
}
