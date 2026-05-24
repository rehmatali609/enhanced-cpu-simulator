// =============================================================================
//  Cache Memory System
//  Demonstrates cache memory concepts with LRU replacement policy
//  Includes: Tag matching, hit/miss tracking, cache statistics
// =============================================================================

#ifndef CACHE_MEMORY_H
#define CACHE_MEMORY_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <climits>

using namespace std;

// Cache Configuration
struct CacheConfig {
    int cacheSize;      // Total cache size in bytes
    int blockSize;      // Cache block size in bytes
    int associativity;  // 1=Direct mapped, >1=Set associative
};

// Cache Entry
struct CacheEntry {
    int tag;            // Memory tag
    int data;           // Cached data value
    bool valid;         // Valid bit
    int accessTime;     // For LRU replacement
};

class CacheMemory {
private:
    vector<vector<CacheEntry>> cache;  // 2D array for set-associative cache
    int numSets;
    int associativity;
    int blockSizeVal;
    int accessCounter;
    
    // Statistics
    int hitCount;
    int missCount;
    int totalAccess;

public:
    // Constructor
    CacheMemory(int cacheSize = 128, int blockSize = 4, int associativity = 2) 
        : associativity(associativity), blockSizeVal(blockSize), 
          accessCounter(0), hitCount(0), missCount(0), totalAccess(0) {
        
        numSets = cacheSize / (blockSize * associativity);
        if (numSets < 1) numSets = 1;
        
        // Initialize cache
        cache.resize(numSets, vector<CacheEntry>(associativity));
        for (int i = 0; i < numSets; i++) {
            for (int j = 0; j < associativity; j++) {
                cache[i][j].valid = false;
                cache[i][j].tag = -1;
                cache[i][j].data = 0;
                cache[i][j].accessTime = 0;
            }
        }
    }

    // Extract cache index from address
    int getIndex(int addr) {
        return (addr / blockSizeVal) % numSets;
    }

    // Extract tag from address
    int getTag(int addr) {
        return addr / (blockSizeVal * numSets);
    }

    // Cache read operation
    bool read(int addr, int& data) {
        totalAccess++;
        int index = getIndex(addr);
        int tag = getTag(addr);

        // Search for tag in the set
        for (int i = 0; i < associativity; i++) {
            if (cache[index][i].valid && cache[index][i].tag == tag) {
                // Cache HIT
                data = cache[index][i].data;
                cache[index][i].accessTime = accessCounter++;
                hitCount++;
                return true;  // Hit
            }
        }

        // Cache MISS
        missCount++;
        return false;  // Miss
    }

    // Cache write operation
    void write(int addr, int data) {
        totalAccess++;
        int index = getIndex(addr);
        int tag = getTag(addr);

        // Search for tag in the set
        for (int i = 0; i < associativity; i++) {
            if (cache[index][i].valid && cache[index][i].tag == tag) {
                // Cache HIT: Update data
                cache[index][i].data = data;
                cache[index][i].accessTime = accessCounter++;
                hitCount++;
                return;
            }
        }

        // Cache MISS: Need to insert
        missCount++;

        // Find empty slot or LRU entry
        int replaceIdx = -1;
        int lruTime = INT_MAX;

        for (int i = 0; i < associativity; i++) {
            if (!cache[index][i].valid) {
                replaceIdx = i;
                break;
            }
            if (cache[index][i].accessTime < lruTime) {
                lruTime = cache[index][i].accessTime;
                replaceIdx = i;
            }
        }

        // Replace cache entry
        if (replaceIdx >= 0 && replaceIdx < (int)cache[index].size()) {
            cache[index][replaceIdx].tag = tag;
            cache[index][replaceIdx].data = data;
            cache[index][replaceIdx].valid = true;
            cache[index][replaceIdx].accessTime = accessCounter++;
        }
    }

    // Get cache statistics
    void displayStats() const {
        cout << "\n  ┌─────────────────────────────────────┐\n";
        cout << "  │         Cache Statistics            │\n";
        cout << "  ├─────────────────────────────────────┤\n";
        cout << "  │ Total Accesses : " << setw(4) << totalAccess << "              │\n";
        cout << "  │ Hits           : " << setw(4) << hitCount << "              │\n";
        cout << "  │ Misses         : " << setw(4) << missCount << "              │\n";
        
        float hitRate = (totalAccess > 0) ? (100.0 * hitCount / totalAccess) : 0;
        float missRate = (totalAccess > 0) ? (100.0 * missCount / totalAccess) : 0;
        
        cout << "  │ Hit Rate       : " << fixed << setprecision(2) 
             << setw(6) << hitRate << " %           │\n";
        cout << "  │ Miss Rate      : " << fixed << setprecision(2) 
             << setw(6) << missRate << " %           │\n";
        cout << "  ├─────────────────────────────────────┤\n";
        cout << "  │ Cache Config:                       │\n";
        cout << "  │   Sets: " << setw(3) << numSets 
             << "  Assoc: " << setw(2) << associativity 
             << "  BlockSize: " << blockSizeVal << "    │\n";
        cout << "  └─────────────────────────────────────┘\n";
    }

    // Reset statistics
    void resetStats() {
        hitCount = 0;
        missCount = 0;
        totalAccess = 0;
    }

    // Get current hit rate
    float getHitRate() const {
        return (totalAccess > 0) ? (100.0 * hitCount / totalAccess) : 0;
    }

    // Get current miss rate
    float getMissRate() const {
        return (totalAccess > 0) ? (100.0 * missCount / totalAccess) : 0;
    }
};

#endif // CACHE_MEMORY_H
