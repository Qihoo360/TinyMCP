#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

namespace mcp {
namespace utils {

/**
 * @brief Thread pool for concurrent task execution
 */
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t num_threads_;

public:
    /**
     * @brief Constructor
     */
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief Destructor
     */
    ~ThreadPool();
    
    /**
     * @brief Add task to the pool
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    /**
     * @brief Get number of threads
     */
    size_t getThreadCount() const { return num_threads_; }
    
    /**
     * @brief Get queue size
     */
    size_t getQueueSize() const;
    
    /**
     * @brief Stop the thread pool
     */
    void stop();
    
    /**
     * @brief Check if pool is stopped
     */
    bool isStopped() const { return stop_.load(); }

private:
    void worker();
};

/**
 * @brief Task executor interface
 */
class TaskExecutor {
public:
    virtual ~TaskExecutor() = default;
    
    /**
     * @brief Execute a task
     */
    virtual void execute(std::function<void()> task) = 0;
    
    /**
     * @brief Execute a task and return future
     */
    virtual std::future<void> executeAsync(std::function<void()> task) = 0;
    
    /**
     * @brief Shutdown the executor
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Check if executor is running
     */
    virtual bool isRunning() const = 0;
};

/**
 * @brief Thread pool task executor
 */
class ThreadPoolTaskExecutor : public TaskExecutor {
private:
    std::unique_ptr<ThreadPool> thread_pool_;
    
public:
    explicit ThreadPoolTaskExecutor(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPoolTaskExecutor();
    
    void execute(std::function<void()> task) override;
    std::future<void> executeAsync(std::function<void()> task) override;
    void shutdown() override;
    bool isRunning() const override;
    
    /**
     * @brief Get thread count
     */
    size_t getThreadCount() const;
    
    /**
     * @brief Get queue size
     */
    size_t getQueueSize() const;
};

/**
 * @brief Single-threaded task executor
 */
class SingleThreadTaskExecutor : public TaskExecutor {
private:
    std::thread worker_thread_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<bool> running_;
    
public:
    SingleThreadTaskExecutor();
    ~SingleThreadTaskExecutor();
    
    void execute(std::function<void()> task) override;
    std::future<void> executeAsync(std::function<void()> task) override;
    void shutdown() override;
    bool isRunning() const override;
    
private:
    void worker();
};

/**
 * @brief Synchronous task executor (executes tasks immediately)
 */
class SyncTaskExecutor : public TaskExecutor {
private:
    std::atomic<bool> running_;
    
public:
    SyncTaskExecutor();
    ~SyncTaskExecutor();
    
    void execute(std::function<void()> task) override;
    std::future<void> executeAsync(std::function<void()> task) override;
    void shutdown() override;
    bool isRunning() const override;
};

/**
 * @brief Task scheduler for delayed and periodic tasks
 */
class TaskScheduler {
private:
    struct ScheduledTask {
        std::function<void()> task;
        std::chrono::steady_clock::time_point execute_time;
        std::chrono::milliseconds interval;
        bool is_periodic;
        std::string id;
        
        ScheduledTask(std::function<void()> t, std::chrono::steady_clock::time_point et, 
                     const std::string& taskId = "")
            : task(std::move(t)), execute_time(et), interval(std::chrono::milliseconds(0)), 
              is_periodic(false), id(taskId) {}
        
        ScheduledTask(std::function<void()> t, std::chrono::steady_clock::time_point et,
                     std::chrono::milliseconds i, const std::string& taskId = "")
            : task(std::move(t)), execute_time(et), interval(i), is_periodic(true), id(taskId) {}
    };
    
    std::unique_ptr<TaskExecutor> executor_;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, 
                       std::function<bool(const ScheduledTask&, const ScheduledTask&)>> tasks_;
    std::mutex tasks_mutex_;
    std::condition_variable condition_;
    std::thread scheduler_thread_;
    std::atomic<bool> stop_;
    
public:
    explicit TaskScheduler(std::unique_ptr<TaskExecutor> executor = nullptr);
    ~TaskScheduler();
    
    /**
     * @brief Schedule a task to run once
     */
    std::string schedule(std::function<void()> task, std::chrono::milliseconds delay,
                        const std::string& taskId = "");
    
    /**
     * @brief Schedule a task to run periodically
     */
    std::string schedulePeriodic(std::function<void()> task, std::chrono::milliseconds interval,
                                const std::string& taskId = "");
    
    /**
     * @brief Schedule a task to run at a specific time
     */
    std::string scheduleAt(std::function<void()> task, std::chrono::steady_clock::time_point time,
                          const std::string& taskId = "");
    
    /**
     * @brief Cancel a scheduled task
     */
    bool cancel(const std::string& taskId);
    
    /**
     * @brief Cancel all scheduled tasks
     */
    void cancelAll();
    
    /**
     * @brief Shutdown the scheduler
     */
    void shutdown();
    
    /**
     * @brief Check if scheduler is running
     */
    bool isRunning() const { return !stop_.load(); }
    
    /**
     * @brief Get number of scheduled tasks
     */
    size_t getScheduledTaskCount() const;

private:
    void schedulerLoop();
    void executeTask(const ScheduledTask& task);
    std::string generateTaskId();
};

/**
 * @brief Task executor factory
 */
class TaskExecutorFactory {
public:
    /**
     * @brief Create thread pool executor
     */
    static std::unique_ptr<TaskExecutor> createThreadPoolExecutor(size_t numThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief Create single-thread executor
     */
    static std::unique_ptr<TaskExecutor> createSingleThreadExecutor();
    
    /**
     * @brief Create synchronous executor
     */
    static std::unique_ptr<TaskExecutor> createSyncExecutor();
    
    /**
     * @brief Create task scheduler
     */
    static std::unique_ptr<TaskScheduler> createTaskScheduler(std::unique_ptr<TaskExecutor> executor = nullptr);
};

// Template implementation
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        tasks_.emplace([task]() { (*task)(); });
    }
    
    condition_.notify_one();
    return res;
}

} // namespace utils
} // namespace mcp