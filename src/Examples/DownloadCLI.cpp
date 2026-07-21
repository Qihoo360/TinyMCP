#include "../Task/DownloadTask.hpp"
#include "../Utils/ThreadSafeQueue.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <memory>

using namespace TinyMCP;

class DownloadCLI {
private:
    Utils::ThreadSafePriorityQueue<std::shared_ptr<Task::DownloadTask>> task_queue_;
    std::atomic<bool> running_{true};
    std::thread worker_thread_;
    std::thread input_thread_;

public:
    DownloadCLI() {
        worker_thread_ = std::thread(&DownloadCLI::workerLoop, this);
        input_thread_ = std::thread(&DownloadCLI::inputLoop, this);
    }

    ~DownloadCLI() {
        running_ = false;
        task_queue_.requestShutdown();
        if (worker_thread_.joinable()) worker_thread_.join();
        if (input_thread_.joinable()) input_thread_.join();
    }

    void run() {
        std::cout << "=== TinyMCP Download CLI ===" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  download <URL> [session_id] - Start/resume download" << std::endl;
        std::cout << "  list - List active sessions" << std::endl;
        std::cout << "  quit - Exit application" << std::endl;
        std::cout << std::endl;

        // Wait for threads to finish
        if (input_thread_.joinable()) input_thread_.join();
        if (worker_thread_.joinable()) worker_thread_.join();
    }

private:
    void inputLoop() {
        std::string line;
        while (running_ && std::getline(std::cin, line)) {
            processCommand(line);
        }
    }

    void workerLoop() {
        std::shared_ptr<Task::DownloadTask> task;
        while (running_ && task_queue_.pop(task)) {
            if (task) {
                std::cout << "Processing download task for session: " << task->getSessionId() << std::endl;
                bool success = task->execute();
                std::cout << "Task " << (success ? "completed successfully" : "failed") 
                         << " for session: " << task->getSessionId() << std::endl;
            }
        }
    }

    void processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "download") {
            std::string url, session_id;
            iss >> url >> session_id;

            if (url.empty()) {
                std::cout << "Error: URL required" << std::endl;
                return;
            }

            auto task = std::make_shared<Task::DownloadTask>(url, session_id);
            task_queue_.push(task, 1); // Normal priority

            std::cout << "Queued download: " << url;
            if (!session_id.empty()) {
                std::cout << " (session: " << session_id << ")";
            } else {
                std::cout << " (new session: " << task->getSessionId() << ")";
            }
            std::cout << std::endl;

        } else if (cmd == "list") {
            Utils::SessionPersistence session_mgr;
            auto sessions = session_mgr.listActiveSessions();
            std::cout << "Active sessions: " << sessions.size() << std::endl;
            for (const auto& id : sessions) {
                std::cout << "  " << id << std::endl;
            }

        } else if (cmd == "quit") {
            std::cout << "Shutting down..." << std::endl;
            running_ = false;
            task_queue_.requestShutdown();

        } else if (!cmd.empty()) {
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    }
};

int main() {
    try {
        DownloadCLI cli;
        cli.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
