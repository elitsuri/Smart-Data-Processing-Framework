#pragma once

#include <queue>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <optional>
#include <chrono>

template<typename T>
class DataQueue {
public:
    explicit DataQueue(size_t maxSize = 10000) 
        : maxSize_(maxSize), shutdown_(false) {}

    ~DataQueue() {
        shutdown();
    }

    // Add item to queue
    bool enqueue(const T& item, int timeoutMs = -1) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (timeoutMs > 0) {
            if (!notFull_.wait_for(lock, 
                std::chrono::milliseconds(timeoutMs),
                [this] { return queue_.size() < maxSize_ && !shutdown_; })) {
                return false; // Timeout
            }
        } else {
            notFull_.wait(lock, [this] { 
                return queue_.size() < maxSize_ && !shutdown_; 
            });
        }

        if (shutdown_) return false;

        queue_.push_back(item);
        notEmpty_.notify_one();
        return true;
    }

    // Retrieve and remove item from queue
    std::optional<T> dequeue(int timeoutMs = -1) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (timeoutMs > 0) {
            if (!notEmpty_.wait_for(lock, 
                std::chrono::milliseconds(timeoutMs),
                [this] { return !queue_.empty() || shutdown_; })) {
                return std::nullopt; // Timeout
            }
        } else {
            notEmpty_.wait(lock, [this] { 
                return !queue_.empty() || shutdown_; 
            });
        }

        if (queue_.empty()) {
            return std::nullopt;
        }

        T item = std::move(queue_.front());
        queue_.pop_front();
        notFull_.notify_one();
        return item;
    }

    // Peek at front item without removing
    std::optional<T> peek() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        return queue_.front();
    }

    // Get queue size
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Check if queue is full
    bool full() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() >= maxSize_;
    }

    // Clear all items
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop_front();
        }
        notFull_.notify_all();
    }

    // Gracefully shutdown queue
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    // Check if shutdown
    bool isShutdown() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return shutdown_;
    }

    // Get statistics
    struct Stats {
        size_t currentSize;
        size_t maxSize;
        bool isFull;
        bool isEmpty;
    };

    Stats getStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {queue_.size(), maxSize_, 
                queue_.size() >= maxSize_, queue_.empty()};
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::deque<T> queue_;
    size_t maxSize_;
    bool shutdown_;
};
