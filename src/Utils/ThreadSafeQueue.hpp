#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

namespace TinyMCP {
namespace Utils {

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable condition_;
    std::atomic<bool> shutdown_{false};

public:
    ThreadSafeQueue() = default;

    // Non-copyable
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!shutdown_) {
            queue_.push(std::move(item));
            condition_.notify_one();
        }
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        condition_.wait(lock, [this] { return !queue_.empty() || shutdown_; });

        if (shutdown_ && queue_.empty()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool tryPop(T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    void requestShutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = true;
        condition_.notify_all();
    }

    bool isShutdown() const {
        return shutdown_.load();
    }
};

// Priority queue variant for message prioritization
template<typename T>
struct PrioritizedItem {
    int priority;
    T item;

    bool operator<(const PrioritizedItem& other) const {
        return priority > other.priority; // Min-heap (lower number = higher priority)
    }
};

template<typename T>
class ThreadSafePriorityQueue {
private:
    mutable std::mutex mtx_;
    std::priority_queue<PrioritizedItem<T>> queue_;
    std::condition_variable condition_;
    std::atomic<bool> shutdown_{false};

public:
    void push(T item, int priority = 1) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!shutdown_) {
            queue_.push({priority, std::move(item)});
            condition_.notify_one();
        }
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        condition_.wait(lock, [this] { return !queue_.empty() || shutdown_; });

        if (shutdown_ && queue_.empty()) {
            return false;
        }

        item = std::move(queue_.top().item);
        queue_.pop();
        return true;
    }

    void requestShutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = true;
        condition_.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
};

} // namespace Utils
} // namespace TinyMCP
