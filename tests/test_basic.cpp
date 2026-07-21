#include "../src/Utils/ThreadSafeQueue.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

using namespace TinyMCP::Utils;

void test_thread_safe_queue() {
    ThreadSafeQueue<int> queue;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};

    // Start producers
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back([&queue, &produced_count, i]() {
            for (int j = 0; j < 10; ++j) {
                queue.push(i * 10 + j);
                produced_count++;
            }
        });
    }

    // Start consumers  
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back([&queue, &consumed_count]() {
            int item;
            while (queue.pop(item)) {
                consumed_count++;
                if (consumed_count >= 30) break;
            }
        });
    }

    // Wait for producers
    for (auto& t : producers) t.join();

    // Shutdown queue after production
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    queue.requestShutdown();

    // Wait for consumers
    for (auto& t : consumers) t.join();

    std::cout << "Produced: " << produced_count << ", Consumed: " << consumed_count << std::endl;
    assert(produced_count == 30);
    assert(consumed_count == 30);
}

int main() {
    std::cout << "Running basic tests..." << std::endl;

    test_thread_safe_queue();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
