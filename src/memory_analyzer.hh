//Memory Analyzer
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class MemoryAnalyzer {
private:
    mutable std::ofstream logFile;
    mutable std::mutex logMutex;
    std::atomic<size_t> activeRegionsCount{0}, activeReferencesCount{0}, activeLinearsCount{0};
    size_t totalAllocated = 0, peakMemoryUsage = 0, allocationCount = 0;
    size_t deallocationCount = 0, largestAllocation = 0;

    struct AllocationMetrics {
        size_t totalSize;
        size_t count;
        size_t peakCount;
        size_t fragmentationScore;
        std::chrono::steady_clock::duration averageLifetime;
        std::vector<size_t> sizeDistribution;
        size_t failedAttempts;
        size_t reallocations;
    };

    struct MemoryBlock {
        void* address;
        size_t size;
        std::chrono::steady_clock::time_point allocationTime;
        std::string stackTrace;
        std::string threadId;
        bool isFreed;
        size_t alignmentPadding;
        size_t accessCount;
        std::vector<std::string> accessPatterns;
    };

    std::unordered_map<void*, MemoryBlock> activeAllocations;
    std::vector<MemoryBlock> historicalAllocations;
    std::map<std::string, AllocationMetrics> metricsPerStackTrace;
   // std::mutex analyzerMutex;
    mutable std::mutex analyzerMutex;

    // Fragmentation tracking
    struct FragmentationInfo {
        size_t totalFragments;
        size_t largestFragment;
        double fragmentationRatio;
        std::vector<size_t> fragmentSizes;
    };

    // Memory access patterns
    struct AccessPattern {
        enum class Type { Sequential, Random, Strided };
        Type type;
        size_t frequency;
        size_t stride;
    };

    // Performance metrics
    struct PerformanceMetrics {
        double averageAllocationTime;
        double averageDeallocationTime;
        size_t cacheHits;
        size_t cacheMisses;
        std::vector<double> allocationLatencies;
    };

    struct TemporalMetrics {
        double peakAllocationRate;
        double averageLifetime;
        std::vector<std::string> hotspots;
    };

    struct ThreadMetrics {
        size_t totalAllocations;
        size_t activeAllocations;
        size_t peakMemoryUsage;
    };

    struct AlignmentMetrics {
        size_t suboptimalCount;
        double averagePaddingWaste;
    };

    struct CacheMetrics {
        size_t hits;
        size_t misses;
        double averageAccessTime;
    };

    struct AccessPatternMetrics {
        std::string description;
        double frequency;
        size_t stride;
    };

    struct HealthMetrics {
        double fragmentationScore;
        double efficiencyScore;
        double cacheScore;
        double safetyScore;
    };

    std::string getTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

public:
    MemoryAnalyzer() {
        logFile.open("memory.log", std::ios::app);
        if (!logFile) {
            throw std::runtime_error("Failed to open memory.log");
        }
        log("MemoryAnalyzer initialized");
    }

    ~MemoryAnalyzer() {
        log("MemoryAnalyzer destroyed");
        logFile.close();
    }

    void recordAllocation(void* ptr, size_t size, const std::string& stackTrace) {
        std::lock_guard<std::mutex> lock(analyzerMutex);

        totalAllocated += size;
        peakMemoryUsage = std::max(peakMemoryUsage, totalAllocated);
        allocationCount++;
        largestAllocation = std::max(largestAllocation, size);

        std::string threadId = getCurrentThreadId();

        MemoryBlock block{
            ptr,
            size,
            std::chrono::steady_clock::now(),
            stackTrace,
            threadId,
            false,
            calculateAlignmentPadding(ptr, size),
            0,
            {}
        };

        activeAllocations[ptr] = block;
        updateMetrics(block);

        log("Allocated " + std::to_string(size) + " bytes at " +
            std::to_string(reinterpret_cast<uintptr_t>(ptr)));
    }

    void recordDeallocation(void* ptr) {
        std::lock_guard<std::mutex> lock(analyzerMutex);

        auto it = activeAllocations.find(ptr);
        if (it != activeAllocations.end()) {
            totalAllocated -= it->second.size;
            deallocationCount++;
            it->second.isFreed = true;
            historicalAllocations.push_back(it->second);
            activeAllocations.erase(it);
        }
    }

    struct AllocationInfo {
        size_t size;
        std::chrono::steady_clock::time_point allocationTime;
        std::string stackTrace;
        std::string threadId;
        bool isFreed;
        size_t alignmentPadding;
        size_t accessCount;
        std::vector<std::string> accessPatterns;
    };

    AllocationInfo* getAllocationInfo(void* ptr) {
        std::lock_guard<std::mutex> lock(analyzerMutex);

        auto it = activeAllocations.find(ptr);
        if (it != activeAllocations.end()) {
            // Convert MemoryBlock to AllocationInfo
            static AllocationInfo info;
            info.size = it->second.size;
            info.allocationTime = it->second.allocationTime;
            info.stackTrace = it->second.stackTrace;
            info.threadId = it->second.threadId;
            info.isFreed = it->second.isFreed;
            info.alignmentPadding = it->second.alignmentPadding;
            info.accessCount = it->second.accessCount;
            info.accessPatterns = it->second.accessPatterns;
            return &info;
        }

        return nullptr;
    }

    struct MemoryUsageReport {
        // Basic metrics
        size_t totalAllocated;
        size_t peakMemoryUsage;
        size_t allocationCount;
        size_t deallocationCount;
        size_t largestAllocation;
        size_t activeRegions;
        size_t activeReferences;
        size_t activeLinears;
        double averageAllocationSize;

        // Statistics report
        std::string statisticsReport;

        // Leak reports
        std::vector<std::string> memoryLeaks;
        std::vector<std::string> detailedLeakReport;

        // Analysis report metrics
        double overallHealth;
        std::vector<std::string> fragmentationIssues;
        std::vector<std::string> performanceWarnings;
        std::vector<std::string> allocationPatterns;

        // Size distribution
        std::map<size_t, size_t> sizeDistribution;

        // Temporal metrics
        double peakAllocationRate;
        double averageLifetime;
        std::vector<std::string> hotspots;

        // Thread metrics
        std::map<std::string, ThreadMetrics> threadMetrics;

        // Alignment metrics
        size_t suboptimalAlignments;
        double averagePaddingWaste;

        // Cache metrics
        double cacheHitRate;
        double averageCacheAccessTime;
        size_t cacheHits;
        size_t cacheMisses;

        // Access patterns
        std::vector<AccessPatternMetrics> accessPatterns;

        // Recommendations
        std::vector<std::string> recommendations;

        // Health breakdown
        double fragmentationScore;
        double efficiencyScore;
        double cacheScore;
        double safetyScore;
    };

    void log(const std::string& message) const {
        std::lock_guard lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] " << message << std::endl;
            logFile.flush();
        }
    }

    MemoryUsageReport getMemoryUsage() const {
        // Use a single lock for the entire method to prevent data races
        std::lock_guard<std::mutex> lock(analyzerMutex);
        MemoryUsageReport report;

        // Capture basic metrics
        report.totalAllocated = totalAllocated;
        report.peakMemoryUsage = peakMemoryUsage;
        report.allocationCount = allocationCount;
        report.deallocationCount = deallocationCount;
        report.largestAllocation = largestAllocation;
        report.activeRegions = activeRegionsCount;
        report.activeReferences = activeReferencesCount;
        report.activeLinears = activeLinearsCount;
        report.averageAllocationSize = allocationCount > 0 ?
                                           static_cast<double>(totalAllocated) / allocationCount : 0;

        // Generate statistics report
        std::stringstream statsStream;
        statsStream << "Memory Manager Statistics:\n"
                    << "  Current Total Allocated: " << formatMemorySize(totalAllocated) << "\n"
                    << "  Peak Memory Usage: " << formatMemorySize(peakMemoryUsage) << "\n"
                    << "  Number of Allocations: " << allocationCount << "\n"
                    << "  Number of Deallocations: " << deallocationCount << "\n"
                    << "  Largest Allocation: " << formatMemorySize(largestAllocation) << "\n"
                    << "  Active Regions: " << activeRegionsCount << "\n"
                    << "  Active References: " << activeReferencesCount << "\n"
                    << "  Active Linears: " << activeLinearsCount << "\n"
                    << "  Average Allocation Size: " << formatMemorySize(report.averageAllocationSize) << "\n";
        report.statisticsReport = statsStream.str();

        // Generate analysis report

        // Memory Leak Analysis
        for (const auto& [ptr, block] : activeAllocations) {
            auto lifetime = std::chrono::steady_clock::now() - block.allocationTime;
            if (std::chrono::duration_cast<std::chrono::hours>(lifetime).count() > 24) {
                std::stringstream ss;
                ss << "Potential memory leak: " << block.size << " bytes at " << ptr
                   << " allocated from " << block.stackTrace
                   << " (age: " << std::chrono::duration_cast<std::chrono::hours>(lifetime).count()
                   << " hours)";
                report.memoryLeaks.push_back(ss.str());
            }
        }

        // Analyze fragmentation
        FragmentationInfo fragInfo = analyzeFragmentation();
        if (fragInfo.fragmentationRatio > 0.3) {
            std::stringstream ss;
            ss << "High memory fragmentation detected: "
               << (fragInfo.fragmentationRatio * 100) << "% fragmentation ratio";
            report.fragmentationIssues.push_back(ss.str());
        }

        // Calculate performance metrics
        PerformanceMetrics perfMetrics = calculatePerformanceMetrics();
        if (perfMetrics.averageAllocationTime > 100) { // microseconds
            report.performanceWarnings.push_back(
                "High average allocation time: " +
                std::to_string(perfMetrics.averageAllocationTime) + " us"
                );
        }

        // Analyze allocation patterns
        auto patterns = analyzeAllocationPatterns();
        for (const auto& pattern : patterns) {
            report.allocationPatterns.push_back(pattern);
        }

        // Overall Health Calculation
        report.overallHealth = calculateOverallHealth(
            fragInfo,
            perfMetrics,
            report.memoryLeaks.size()
            );

        // Detailed leak report
        for (const auto& [ptr, block] : activeAllocations) {
            auto duration = std::chrono::steady_clock::now() - block.allocationTime;
            std::stringstream leakStream;
            leakStream << "Leak: " << formatMemorySize(block.size) << " at "
                       << std::hex << std::showbase << reinterpret_cast<uintptr_t>(ptr)
                       << std::dec << "\n"
                       << "    Age: " << std::chrono::duration_cast<std::chrono::seconds>(duration).count()
                       << " seconds\n"
                       << "    Stack Trace: " << block.stackTrace << "\n"
                       << "    Thread ID: " << block.threadId << "\n"
                       << "    Access Count: " << block.accessCount << "\n"
                       << "    Access Patterns: " << block.accessPatterns.size() << " recorded";
            report.detailedLeakReport.push_back(leakStream.str());
        }

        // Size distribution
        report.sizeDistribution = getSizeDistribution();

        // Temporal metrics
        auto temporalMetrics = getTemporalMetrics();
        report.peakAllocationRate = temporalMetrics.peakAllocationRate;
        report.averageLifetime = temporalMetrics.averageLifetime;
        report.hotspots = temporalMetrics.hotspots;

        // Thread metrics
        report.threadMetrics = getThreadMetrics();

        // Alignment metrics
        auto alignmentMetrics = getAlignmentMetrics();
        report.suboptimalAlignments = alignmentMetrics.suboptimalCount;
        report.averagePaddingWaste = alignmentMetrics.averagePaddingWaste;

        // Cache metrics
        auto cacheMetrics = getCacheMetrics();
        report.cacheHits = cacheMetrics.hits;
        report.cacheMisses = cacheMetrics.misses;
        report.cacheHitRate = (cacheMetrics.hits + cacheMetrics.misses > 0) ?
                                  (cacheMetrics.hits * 100.0) / (cacheMetrics.hits + cacheMetrics.misses) : 0.0;
        report.averageCacheAccessTime = cacheMetrics.averageAccessTime;

        // Access patterns
        report.accessPatterns = getAccessPatterns();

        // Recommendations
        report.recommendations = getRecommendations();

        // Health metrics
        auto healthMetrics = getHealthMetrics();
        report.fragmentationScore = healthMetrics.fragmentationScore;
        report.efficiencyScore = healthMetrics.efficiencyScore;
        report.cacheScore = healthMetrics.cacheScore;
        report.safetyScore = healthMetrics.safetyScore;

        return report;
    }

    // Complete printMemoryUsageReport implementation
    void printMemoryUsageReport(const MemoryUsageReport& report) const {
        std::cout << "\n \n\n";
        std::cout << "\n=== Memory Usage Report ===\n\n";

        // Print Statistics
        std::cout << report.statisticsReport << "\n";

        // Print Memory Leaks
        if (!report.detailedLeakReport.empty()) {
            std::cout << "=== Detailed Memory Leak Report ===\n";
            for (const auto& leak : report.detailedLeakReport) {
                std::cout << leak << "\n";
            }
            std::cout << "\n";
        }

        // Print Analysis Results
        std::cout << "=== Memory Analysis ===\n";
        std::cout << "Overall Health Score: " << std::fixed << std::setprecision(1)
                  << report.overallHealth << "/100\n\n";

        // Print Fragmentation Issues
        if (!report.fragmentationIssues.empty()) {
            std::cout << "Fragmentation Issues:\n";
            std::cout << std::string(20, '-') << "\n";
            for (const auto& issue : report.fragmentationIssues) {
                std::cout << "- " << issue << "\n";
            }
            std::cout << "\n";
        }

        // Print Performance Warnings
        if (!report.performanceWarnings.empty()) {
            std::cout << "Performance Warnings:\n";
            std::cout << std::string(20, '-') << "\n";
            for (const auto& warning : report.performanceWarnings) {
                std::cout << "- " << warning << "\n";
            }
            std::cout << "\n";
        }

        // Print Size Distribution
        std::cout << "Allocation Size Distribution:\n";
        std::cout << std::string(20, '-') << "\n";
        if (report.sizeDistribution.empty()) {
            std::cout << "No allocations recorded\n";
        } else {
            for (const auto& [sizeClass, count] : report.sizeDistribution) {
                std::cout << std::setw(8) << formatMemorySize(sizeClass) << ": "
                          << std::string(count / 10, '|') << " " << count << "\n";
            }
        }
        std::cout << "\n";

        // Print Temporal Analysis
        std::cout << "Temporal Analysis:\n";
        std::cout << std::string(20, '-') << "\n";
        std::cout << "Peak Allocation Rate: " << std::fixed << std::setprecision(1)
                  << report.peakAllocationRate << " allocs/sec\n";
        std::cout << "Average Allocation Lifetime: " << std::fixed << std::setprecision(1)
                  << report.averageLifetime << " ms\n";

        if (!report.hotspots.empty()) {
            std::cout << "\nAllocation Hotspots:\n";
            for (const auto& hotspot : report.hotspots) {
                std::cout << "- " << hotspot << "\n";
            }
        }
        std::cout << "\n";

        // Print Thread Analysis
        std::cout << "Thread Analysis:\n";
        std::cout << std::string(20, '-') << "\n";
        for (const auto& [threadId, metrics] : report.threadMetrics) {
            std::cout << "Thread " << threadId << ":\n"
                      << "  Total Allocations: " << metrics.totalAllocations << "\n"
                      << "  Active Allocations: " << metrics.activeAllocations << "\n"
                      << "  Peak Memory Usage: " << formatMemorySize(metrics.peakMemoryUsage) << "\n\n";
        }

        // Print Cache Performance
        std::cout << "Cache Performance:\n";
        std::cout << std::string(20, '-') << "\n";
        std::cout << "Cache Hit Rate: " << std::fixed << std::setprecision(1)
                  << report.cacheHitRate << "%\n"
                  << "Cache Hits: " << report.cacheHits << "\n"
                  << "Cache Misses: " << report.cacheMisses << "\n"
                  << "Average Access Time: " << std::fixed << std::setprecision(1)
                  << report.averageCacheAccessTime << " ns\n\n";

        // Print Access Patterns
        std::cout << "Memory Access Patterns:\n";
        std::cout << std::string(20, '-') << "\n";
        if (report.accessPatterns.empty()) {
            std::cout << "No access patterns recorded\n";
        } else {
            for (const auto& pattern : report.accessPatterns) {
                std::cout << "- " << pattern.description << "\n";
            }
        }
        std::cout << "\n";

        // Print Recommendations
        std::cout << "Recommendations:\n";
        std::cout << std::string(20, '-') << "\n";
        for (const auto& rec : report.recommendations) {
            std::cout << "- " << rec << "\n";
        }
        std::cout << "\n";

        // Print Health Score Breakdown
        std::cout << "Health Score Breakdown:\n";
        std::cout << std::string(20, '-') << "\n";
        std::cout << "Memory Fragmentation: " << std::fixed << std::setprecision(1)
                  << report.fragmentationScore << "/100\n"
                  << "Allocation Efficiency: " << std::fixed << std::setprecision(1)
                  << report.efficiencyScore << "/100\n"
                  << "Cache Utilization: " << std::fixed << std::setprecision(1)
                  << report.cacheScore << "/100\n"
                  << "Memory Safety: " << std::fixed << std::setprecision(1)
                  << report.safetyScore << "/100\n";

        std::cout << "\n" << std::string(50, '=') << "\n";
    }

    // Add accessors for the atomic counters
    void incrementActiveRegions() { activeRegionsCount++; }
    void decrementActiveRegions() { activeRegionsCount--; }
    void incrementActiveReferences() { activeReferencesCount++; }
    void decrementActiveReferences() { activeReferencesCount--; }
    void incrementActiveLinears() { activeLinearsCount++; }
    void decrementActiveLinears() { activeLinearsCount--; }

    void recordAccess(void* ptr, size_t offset, size_t size) {
        std::lock_guard<std::mutex> lock(analyzerMutex);

        auto it = activeAllocations.find(ptr);
        if (it != activeAllocations.end()) {
            it->second.accessCount++;
            recordAccessPattern(it->second, offset, size);
        }
    }

    // New methods to support detailed analysis
    std::map<size_t, size_t> getSizeDistribution() const {
        std::map<size_t, size_t> distribution;
        for (const auto& [_, block] : activeAllocations) {
            distribution[getSizeClass(block.size)]++;
        }
        return distribution;
    }

    TemporalMetrics getTemporalMetrics() const {
        TemporalMetrics metrics{};

        // Calculate allocation rate
        if (!historicalAllocations.empty()) {
            auto timeSpan = historicalAllocations.back().allocationTime -
                            historicalAllocations.front().allocationTime;
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(timeSpan);
            metrics.peakAllocationRate = historicalAllocations.size() /
                                         (duration.count() > 0 ? duration.count() : 1.0);
        }

        // Calculate average lifetime
        double totalLifetime = 0;
        size_t count = 0;
        for (const auto& alloc : historicalAllocations) {
            if (alloc.isFreed) {
                auto lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::steady_clock::now() - alloc.allocationTime).count();
                totalLifetime += lifetime;
                count++;
            }
        }
        metrics.averageLifetime = count > 0 ? totalLifetime / count : 0;

        // Identify hotspots
        std::map<std::string, size_t> stackTraceCount;
        for (const auto& alloc : historicalAllocations) {
            stackTraceCount[alloc.stackTrace]++;
        }

        for (const auto& [trace, count] : stackTraceCount) {
            if (count > historicalAllocations.size() / 10) {  // 10% threshold
                metrics.hotspots.push_back(trace);
            }
        }

        return metrics;
    }

    std::map<std::string, ThreadMetrics> getThreadMetrics() const {
        std::map<std::string, ThreadMetrics> metrics;

        // Process active allocations
        for (const auto& [_, block] : activeAllocations) {
            auto& threadMetrics = metrics[block.threadId];
            threadMetrics.activeAllocations++;
            threadMetrics.totalAllocations++;
            threadMetrics.peakMemoryUsage = std::max(
                threadMetrics.peakMemoryUsage,
                block.size
                );
        }

        // Process historical allocations
        for (const auto& block : historicalAllocations) {
            auto& threadMetrics = metrics[block.threadId];
            threadMetrics.totalAllocations++;
            threadMetrics.peakMemoryUsage = std::max(
                threadMetrics.peakMemoryUsage,
                block.size
                );
        }

        return metrics;
    }

    AlignmentMetrics getAlignmentMetrics() const {
        AlignmentMetrics metrics{};
        double totalWaste = 0;
        size_t count = 0;

        for (const auto& [_, block] : activeAllocations) {
            if (block.alignmentPadding > 0) {
                metrics.suboptimalCount++;
                totalWaste += block.alignmentPadding;
                count++;
            }
        }

        metrics.averagePaddingWaste = count > 0 ? totalWaste / count : 0;
        return metrics;
    }

    CacheMetrics getCacheMetrics() const {
        CacheMetrics metrics{};

        // Initialize with default values
        metrics.hits = 0;
        metrics.misses = 0;
        metrics.averageAccessTime = 1.0;  // baseline latency in ns

        bool hasAccessPatterns = false;

        for (const auto& [_, block] : activeAllocations) {
            if (block.accessPatterns.size() >= 2) {
                hasAccessPatterns = true;
                for (size_t i = 1; i < block.accessPatterns.size(); i++) {
                    if (isSequentialAccess(block.accessPatterns[i-1], block.accessPatterns[i])) {
                        metrics.hits++;
                    } else {
                        metrics.misses++;
                    }
                }
            }
        }

        // Add at least one hit if we have access patterns but no hits/misses
        // This prevents the NaN in hit rate calculation
        if (hasAccessPatterns && (metrics.hits == 0 && metrics.misses == 0)) {
            metrics.hits = 1;
        }

        double totalAccesses = metrics.hits + metrics.misses;
        if (totalAccesses > 0) {
            metrics.averageAccessTime = (metrics.hits * 1.0 + metrics.misses * 10.0) / totalAccesses;
        }

        return metrics;
    }

    std::vector<AccessPatternMetrics> getAccessPatterns() const {
        std::vector<AccessPatternMetrics> patterns;

        for (const auto& [_, block] : activeAllocations) {
            if (block.accessPatterns.size() < 2) continue;

            AccessPatternMetrics pattern;
            pattern.stride = detectStride(block.accessPatterns);
            pattern.frequency = calculateAccessFrequency(block);

            // Enhanced pattern description
            std::stringstream ss;
            if (pattern.stride > 0) {
                ss << "Regular stride pattern detected: " << pattern.stride << " bytes";
                if (pattern.stride % 64 == 0) {  // Cache line aligned
                    ss << " (cache line aligned)";
                }
            } else {
                ss << "Random access pattern";
            }
            ss << " with " << std::fixed << std::setprecision(2)
               << pattern.frequency << " accesses/sec";

            pattern.description = ss.str();
            patterns.push_back(pattern);
        }

        // Add default pattern if none detected
        if (patterns.empty() && !activeAllocations.empty()) {
            AccessPatternMetrics defaultPattern;
            defaultPattern.stride = 0;
            defaultPattern.frequency = 0;
            defaultPattern.description = "No consistent access pattern detected";
            patterns.push_back(defaultPattern);
        }

        return patterns;
    }

    std::vector<std::string> getRecommendations() const {
        std::vector<std::string> recommendations;

        // Check fragmentation
        auto fragInfo = analyzeFragmentation();
        if (fragInfo.fragmentationRatio > 0.3) {
            recommendations.push_back("Consider implementing a memory pool for frequently allocated sizes");
        }

        // Check alignment
        auto alignMetrics = getAlignmentMetrics();
        if (alignMetrics.suboptimalCount > activeAllocations.size() / 4) {
            recommendations.push_back("Review allocation alignment requirements to reduce padding waste");
        }

        // Check allocation time
        auto perfMetrics = calculatePerformanceMetrics();
        if (perfMetrics.averageAllocationTime > 100) {  // More than 100 microseconds
            recommendations.push_back(
                "High allocation times detected. Consider using a memory pool or pre-allocation strategy"
                );
        }

        // Check memory lifetime
        auto temporalMetrics = getTemporalMetrics();
        if (temporalMetrics.averageLifetime < 1.0) {  // Very short-lived allocations
            recommendations.push_back(
                "Detected many short-lived allocations. Consider using an object pool or stack allocation"
                );
        }

        // If no specific issues found, provide a general recommendation
        if (recommendations.empty()) {
            recommendations.push_back("Memory usage patterns appear optimal. Continue monitoring for changes.");
        }

        return recommendations;
    }

    HealthMetrics getHealthMetrics() const {
        HealthMetrics metrics{};

        // Fragmentation score
        auto fragInfo = analyzeFragmentation();
        metrics.fragmentationScore = 100 * (1.0 - fragInfo.fragmentationRatio);

        // Efficiency score
        auto alignMetrics = getAlignmentMetrics();
        double wasteRatio = alignMetrics.averagePaddingWaste /
                            (activeAllocations.empty() ? 1.0 : activeAllocations.size());
        metrics.efficiencyScore = 100 * (1.0 - std::min(1.0, wasteRatio / 64.0));

        // Cache score
        auto cacheMetrics = getCacheMetrics();
        double totalAccesses = cacheMetrics.hits + cacheMetrics.misses;
        metrics.cacheScore = totalAccesses > 0 ?
                                 (cacheMetrics.hits * 100.0 / totalAccesses) : 100.0;

        // Safety score
        metrics.safetyScore = 100.0;
        for (const auto& [_, block] : activeAllocations) {
            auto lifetime = std::chrono::steady_clock::now() - block.allocationTime;
            if (std::chrono::duration_cast<std::chrono::hours>(lifetime).count() > 24) {
                metrics.safetyScore -= 5;
            }
        }
        metrics.safetyScore = std::max(0.0, metrics.safetyScore);

        return metrics;
    }

private:
    std::string getCurrentThreadId() const {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return ss.str();
    }

    size_t calculateAlignmentPadding(void* ptr, size_t size) const {
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        return address % alignof(std::max_align_t);
    }

    void recordAccessPattern(MemoryBlock& block, size_t offset, size_t size) {
        std::stringstream ss;
        ss << "Access at offset " << offset << " size " << size;
        block.accessPatterns.push_back(ss.str());
    }

    void updateMetrics(const MemoryBlock& block) {
        auto& metrics = metricsPerStackTrace[block.stackTrace];
        metrics.totalSize += block.size;
        metrics.count++;
        metrics.peakCount = std::max(metrics.peakCount, metrics.count);

        // Update size distribution
        size_t sizeClass = getSizeClass(block.size);
        if (sizeClass >= metrics.sizeDistribution.size()) {
            metrics.sizeDistribution.resize(sizeClass + 1);
        }
        metrics.sizeDistribution[sizeClass]++;
    }

    size_t getSizeClass(size_t size) const {
        return static_cast<size_t>(std::log(size));
    }

    FragmentationInfo analyzeFragmentation() const {
        FragmentationInfo info{};

        std::vector<std::pair<uintptr_t, uintptr_t>> addressRanges;
        for (const auto& [ptr, block] : activeAllocations) {
            uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
            uintptr_t end = start + block.size;
            addressRanges.emplace_back(start, end);
        }

        std::sort(addressRanges.begin(), addressRanges.end());

        for (size_t i = 1; i < addressRanges.size(); ++i) {
            size_t gap = addressRanges[i].first - addressRanges[i-1].second;
            if (gap > 0) {
                info.totalFragments++;
                info.fragmentSizes.push_back(gap);
                info.largestFragment = std::max(info.largestFragment, gap);
            }
        }

        size_t totalMemory = 0;
        size_t totalGaps = 0;
        for (const auto& [_, block] : activeAllocations) {
            totalMemory += block.size;
        }
        for (size_t gap : info.fragmentSizes) {
            totalGaps += gap;
        }

        info.fragmentationRatio = totalMemory > 0 ?
                                      static_cast<double>(totalGaps) / totalMemory : 0.0;

        return info;
    }

    std::vector<std::string> analyzeAllocationPatterns() const {
        std::vector<std::string> patterns;

        // Analyze temporal patterns
        std::map<std::string, std::vector<std::chrono::steady_clock::time_point>> allocationTimes;
        for (const auto& allocation : historicalAllocations) {
            allocationTimes[allocation.stackTrace].push_back(allocation.allocationTime);
        }

        for (const auto& [stackTrace, times] : allocationTimes) {
            if (times.size() < 2) continue;

            std::vector<std::chrono::steady_clock::duration> intervals;
            for (size_t i = 1; i < times.size(); ++i) {
                intervals.push_back(times[i] - times[i-1]);
            }

            if (isPeriodicPattern(intervals)) {
                patterns.push_back("Periodic allocation pattern detected at: " + stackTrace);
            }
        }

        // Analyze size patterns
        std::map<std::string, std::vector<size_t>> sizesPerTrace;
        for (const auto& [trace, metrics] : metricsPerStackTrace) {
            if (hasGeometricProgression(metrics.sizeDistribution)) {
                patterns.push_back("Geometric size progression detected at: " + trace);
            }
        }

        return patterns;
    }

    bool isPeriodicPattern(const std::vector<std::chrono::steady_clock::duration>& intervals) const {
        if (intervals.size() < 3) return false;

        auto avgInterval = std::accumulate(intervals.begin(), intervals.end(),
                                           std::chrono::steady_clock::duration{}) / intervals.size();

        int similarIntervals = 0;
        for (const auto& interval : intervals) {
            if (std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(interval - avgInterval).count()) < 100) {
                similarIntervals++;
            }
        }

        return similarIntervals > intervals.size() * 0.8;
    }

    bool hasGeometricProgression(const std::vector<size_t>& sizes) const {
        if (sizes.size() < 3) return false;

        std::vector<double> ratios;
        for (size_t i = 1; i < sizes.size(); ++i) {
            if (sizes[i-1] == 0) continue;
            ratios.push_back(static_cast<double>(sizes[i]) / sizes[i-1]);
        }

        if (ratios.empty()) return false;

        double avgRatio = std::accumulate(ratios.begin(), ratios.end(), 0.0) / ratios.size();
        int similarRatios = 0;
        for (double ratio : ratios) {
            if (std::abs(ratio - avgRatio) < 0.1) {
                similarRatios++;
            }
        }

        return similarRatios > ratios.size() * 0.8;
    }

    PerformanceMetrics calculatePerformanceMetrics() const {
        PerformanceMetrics metrics{};

        for (const auto& allocation : historicalAllocations) {
            auto lifetime = allocation.isFreed ?
                                (std::chrono::steady_clock::now() - allocation.allocationTime) :
                                std::chrono::steady_clock::duration::zero();

            metrics.allocationLatencies.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(lifetime).count()
                );
        }

        if (!metrics.allocationLatencies.empty()) {
            metrics.averageAllocationTime =
                std::accumulate(metrics.allocationLatencies.begin(),
                                metrics.allocationLatencies.end(), 0.0) /
                metrics.allocationLatencies.size();
        }

        return metrics;
    }

    double calculateOverallHealth(
        const FragmentationInfo& fragInfo,
        const PerformanceMetrics& perfMetrics,
        size_t leakCount) const {

        double healthScore = 100.0;

        // Penalize for fragmentation
        healthScore -= fragInfo.fragmentationRatio * 30;

        // Penalize for memory leaks
        healthScore -= std::min(leakCount * 10.0, 30.0);

        // Penalize for poor performance
        double perfPenalty = std::min((perfMetrics.averageAllocationTime - 100) / 10.0, 20.0);
        if (perfPenalty > 0) {
            healthScore -= perfPenalty;
        }

        return std::max(0.0, std::min(100.0, healthScore));
    }

    bool isSequentialAccess(const std::string& prev, const std::string& current) const {
        // Extract offsets from the access pattern strings
        // Expected format: "Access at offset X size Y"
        size_t prevOffset = 0, prevSize = 0;
        size_t currOffset = 0, currSize = 0;

        std::istringstream prevStream(prev);
        std::istringstream currStream(current);
        std::string temp;

        // Parse previous access
        prevStream >> temp >> temp >> temp >> prevOffset >> temp >> prevSize;

        // Parse current access
        currStream >> temp >> temp >> temp >> currOffset >> temp >> currSize;

        // Check if current access starts where previous access ended
        // or has a small overlap/gap (within one cache line)
        const size_t cacheLineSize = 64;  // typical cache line size
        size_t prevEnd = prevOffset + prevSize;

        // Calculate the gap or overlap between accesses
        std::ptrdiff_t gap = static_cast<std::ptrdiff_t>(currOffset) -
                             static_cast<std::ptrdiff_t>(prevEnd);

        // Consider access sequential if:
        // 1. It starts exactly where previous ended (gap == 0)
        // 2. There's a small gap less than a cache line
        // 3. There's a small overlap less than half the previous access
        return gap == 0 ||
               (gap > 0 && static_cast<size_t>(gap) <= cacheLineSize) ||
               (gap < 0 && static_cast<size_t>(-gap) <= prevSize / 2);
    }

    size_t detectStride(const std::vector<std::string>& patterns) const {
        if (patterns.size() < 2) {
            return 0;  // Not enough patterns to detect stride
        }

        std::vector<std::ptrdiff_t> strides;
        size_t prevOffset = 0;
        std::string temp;

        // Calculate strides between consecutive accesses
        for (const auto& pattern : patterns) {
            std::istringstream ss(pattern);
            size_t currentOffset = 0;

            // Parse "Access at offset X size Y"
            ss >> temp >> temp >> temp >> currentOffset;

            if (!strides.empty()) {
                std::ptrdiff_t stride = static_cast<std::ptrdiff_t>(currentOffset) -
                                        static_cast<std::ptrdiff_t>(prevOffset);
                strides.push_back(stride);
            }

            prevOffset = currentOffset;
        }

        if (strides.empty()) {
            return 0;
        }

        // Count frequency of each stride
        std::map<std::ptrdiff_t, size_t> strideFrequency;
        for (std::ptrdiff_t stride : strides) {
            strideFrequency[stride]++;
        }

        // Find the most common stride
        std::ptrdiff_t mostCommonStride = 0;
        size_t maxFrequency = 0;

        for (const auto& [stride, frequency] : strideFrequency) {
            if (frequency > maxFrequency) {
                maxFrequency = frequency;
                mostCommonStride = stride;
            }
        }

        // Only consider it a pattern if it occurs in at least 75% of accesses
        double patternConfidence = static_cast<double>(maxFrequency) / strides.size();
        if (patternConfidence >= 0.75) {
            return static_cast<size_t>(std::abs(mostCommonStride));
        }

        return 0;  // No consistent stride pattern detected
    }

    double calculateAccessFrequency(const MemoryBlock& block) const {
        auto timeSpan = std::chrono::steady_clock::now() - block.allocationTime;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(timeSpan).count();
        return duration > 0 ? block.accessCount / static_cast<double>(duration) : 0;
    }

    std::string generatePatternDescription(size_t stride, double frequency) const {
        std::stringstream ss;
        ss << "Stride: " << stride << " bytes, Frequency: " << frequency << " accesses/sec";
        return ss.str();
    }

    // Add a helper method to format memory sizes
    std::string formatMemorySize(size_t bytes) const {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 3) {
            size /= 1024.0;
            unitIndex++;
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return ss.str();
    }
};
