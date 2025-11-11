#pragma once

#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "DataQueue.h"
#include "Processor.h"
#include "ProcessorFactory.h"
#include "Logger.h"

template<typename T>
class ProcessingSystem {
public:
    explicit ProcessingSystem(size_t numWorkers = 4, size_t queueSize = 10000)
        : numWorkers_(numWorkers), 
          inputQueue_(queueSize),
          outputQueue_(queueSize),
          isRunning_(false),
          totalProcessed_(0),
          totalErrors_(0) {
        LOG_INFO("ProcessingSystem initialized with " + std::to_string(numWorkers) + " workers");
    }

    ~ProcessingSystem() {
        stop();
    }

    // Delete copy semantics - RAII principle
    ProcessingSystem(const ProcessingSystem&) = delete;
    ProcessingSystem& operator=(const ProcessingSystem&) = delete;

    // Allow move semantics
    ProcessingSystem(ProcessingSystem&& other) noexcept = default;
    ProcessingSystem& operator=(ProcessingSystem&& other) noexcept = default;

    // Start the processing system
    void start() {
        if (isRunning_.exchange(true)) {
            LOG_WARNING("System already running");
            return;
        }

        if (!processor_) {
            LOG_ERROR("No processor assigned. Use setProcessor() first.");
            isRunning_ = false;
            return;
        }

        LOG_INFO("Starting ProcessingSystem with " + std::to_string(numWorkers_) + " worker threads");

        // Create worker threads
        for (size_t i = 0; i < numWorkers_; ++i) {
            workers_.emplace_back(&ProcessingSystem::workerThread, this, i);
        }
    }

    // Stop the processing system
    void stop() {
        if (!isRunning_.exchange(false)) {
            return;
        }

        LOG_INFO("Stopping ProcessingSystem");

        inputQueue_.shutdown();

        // Wait for all workers to finish
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers_.clear();

        LOG_INFO("ProcessingSystem stopped. Total processed: " + 
                std::to_string(totalProcessed_) + 
                ", Errors: " + std::to_string(totalErrors_));
    }

    // Set the processor to use
    void setProcessor(std::shared_ptr<Processor<T>> processor) {
        std::lock_guard<std::mutex> lock(processorMutex_);
        processor_ = processor;
        LOG_INFO("Processor set: " + processor_->getName());
    }

    // Set processor by type and parameters (using Factory)
    void setProcessorByType(ProcessorType type, 
                           const std::map<std::string, double>& params = {}) {
        auto processor = ProcessorFactory<T>::getInstance()
            .createProcessor(type, params);
        setProcessor(processor);
    }

    // Add data to processing queue
    bool addData(const T& data, int timeoutMs = 1000) {
        if (!isRunning_) {
            LOG_WARNING("System not running. Cannot add data.");
            return false;
        }
        return inputQueue_.enqueue(data, timeoutMs);
    }

    // Get processed data from output queue
    std::optional<T> getResult(int timeoutMs = 1000) {
        return outputQueue_.dequeue(timeoutMs);
    }

    // Get multiple results
    std::vector<T> getResults(size_t count, int timeoutMs = 100) {
        std::vector<T> results;
        for (size_t i = 0; i < count; ++i) {
            auto result = outputQueue_.dequeue(timeoutMs);
            if (result) {
                results.push_back(result.value());
            }
        }
        return results;
    }

    // System statistics
    struct Statistics {
        size_t inputQueueSize;
        size_t outputQueueSize;
        size_t totalProcessed;
        size_t totalErrors;
        bool isRunning;
        std::string processorName;
    };

    Statistics getStatistics() const {
        std::lock_guard<std::mutex> lock(processorMutex_);
        return {
            inputQueue_.size(),
            outputQueue_.size(),
            totalProcessed_.load(),
            totalErrors_.load(),
            isRunning_.load(),
            processor_ ? processor_->getName() : "None"
        };
    }

    // Print detailed statistics
    void printStatistics() const {
        auto stats = getStatistics();
        LOG_INFO("=== System Statistics ===");
        LOG_INFO("Status: " + std::string(stats.isRunning ? "RUNNING" : "STOPPED"));
        LOG_INFO("Processor: " + stats.processorName);
        LOG_INFO("Input Queue: " + std::to_string(stats.inputQueueSize));
        LOG_INFO("Output Queue: " + std::to_string(stats.outputQueueSize));
        LOG_INFO("Total Processed: " + std::to_string(stats.totalProcessed));
        LOG_INFO("Total Errors: " + std::to_string(stats.totalErrors));
    }

private:
    void workerThread(size_t workerId) {
        LOG_INFO("Worker thread " + std::to_string(workerId) + " started");

        while (isRunning_.load() || !inputQueue_.empty()) {
            auto item = inputQueue_.dequeue(500);
            
            if (!item) {
                continue;
            }

            try {
                std::lock_guard<std::mutex> lock(processorMutex_);
                
                if (!processor_) {
                    LOG_ERROR("Processor not available in worker " + 
                             std::to_string(workerId));
                    totalErrors_++;
                    continue;
                }

                T result = processor_->process(item.value());
                
                if (outputQueue_.enqueue(result, 500)) {
                    totalProcessed_++;
                } else {
                    LOG_WARNING("Failed to enqueue result in worker " + 
                               std::to_string(workerId));
                    totalErrors_++;
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Worker " + std::to_string(workerId) + 
                         " exception: " + std::string(e.what()));
                totalErrors_++;
            }
        }

        LOG_INFO("Worker thread " + std::to_string(workerId) + " finished");
    }

    size_t numWorkers_;
    DataQueue<T> inputQueue_;
    DataQueue<T> outputQueue_;
    
    std::vector<std::thread> workers_;
    std::atomic<bool> isRunning_;
    
    std::shared_ptr<Processor<T>> processor_;
    mutable std::mutex processorMutex_;
    
    std::atomic<size_t> totalProcessed_;
    std::atomic<size_t> totalErrors_;
};
