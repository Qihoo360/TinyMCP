#include "../MCP/MCPServerWrapper.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace TinyMCP::MCP;

class MCPServerManagerCLI {
private:
    std::unique_ptr<MCPServerWrapper> server_wrapper_;
    std::atomic<bool> running_{true};

public:
    MCPServerManagerCLI() {
        server_wrapper_ = std::make_unique<MCPServerWrapper>();
    }

    ~MCPServerManagerCLI() = default;

    void run() {
        std::cout << "=== TinyMCP Server Management CLI ===" << std::endl;
        std::cout << "Enhanced with MCP-specific download capabilities" << std::endl;
        std::cout << std::endl;

        while (running_) {
            displayMenu();
            std::string command;
            std::getline(std::cin, command);
            
            if (!processCommand(command)) {
                break;
            }
        }
    }

private:
    void displayMenu() {
        std::cout << "\n--- MCP Server Management Menu ---" << std::endl;
        std::cout << "Download Operations:" << std::endl;
        std::cout << "  1. download <URL> [destination] - Download package" << std::endl;
        std::cout << "  2. github <owner/repo> [version] - Download GitHub repo" << std::endl;
        std::cout << "  3. docker <image> [tag] - Download Docker image" << std::endl;
        std::cout << "  4. npm <package> [version] - Install NPM package" << std::endl;
        std::cout << std::endl;
        std::cout << "Server Management:" << std::endl;
        std::cout << "  5. install <server> [version] - Install MCP server" << std::endl;
        std::cout << "  6. uninstall <server> - Uninstall MCP server" << std::endl;
        std::cout << "  7. update <server> [version] - Update MCP server" << std::endl;
        std::cout << "  8. list - List installed servers" << std::endl;
        std::cout << "  9. search [query] - Search available servers" << std::endl;
        std::cout << std::endl;
        std::cout << "Runtime Management:" << std::endl;
        std::cout << "  10. start <server> - Start MCP server" << std::endl;
        std::cout << "  11. stop <server> - Stop MCP server" << std::endl;
        std::cout << "  12. restart <server> - Restart MCP server" << std::endl;
        std::cout << "  13. status [server] - Get server status" << std::endl;
        std::cout << std::endl;
        std::cout << "Monitoring:" << std::endl;
        std::cout << "  14. progress <job_id> - Get download progress" << std::endl;
        std::cout << "  15. logs <server> [lines] - Get server logs" << std::endl;
        std::cout << "  16. config <server> - Get server configuration" << std::endl;
        std::cout << std::endl;
        std::cout << "  17. quit - Exit application" << std::endl;
        std::cout << std::endl;
        std::cout << "Enter command: ";
    }

    bool processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit" || cmd == "exit") {
            std::cout << "Shutting down..." << std::endl;
            running_ = false;
            return false;
        }

        try {
            if (cmd == "download") {
                std::string url, destination;
                iss >> url >> destination;
                if (url.empty()) {
                    std::cout << "Error: URL required" << std::endl;
                    return true;
                }
                std::cout << "Downloading: " << url << std::endl;
                std::string result = server_wrapper_->downloadPackage(url, destination);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "github") {
                std::string repo, version, destination;
                iss >> repo >> version >> destination;
                if (repo.empty()) {
                    std::cout << "Error: Repository required (owner/repo)" << std::endl;
                    return true;
                }
                std::cout << "Downloading GitHub repo: " << repo << std::endl;
                std::string result = server_wrapper_->downloadGitHubRepo(repo, destination, version);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "docker") {
                std::string image, tag, destination;
                iss >> image >> tag >> destination;
                if (image.empty()) {
                    std::cout << "Error: Image name required" << std::endl;
                    return true;
                }
                std::cout << "Downloading Docker image: " << image << std::endl;
                std::string result = server_wrapper_->downloadDockerImage(image, tag);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "npm") {
                std::string package, version;
                iss >> package >> version;
                if (package.empty()) {
                    std::cout << "Error: Package name required" << std::endl;
                    return true;
                }
                std::cout << "Installing NPM package: " << package << std::endl;
                std::string result = server_wrapper_->installNPMPackage(package, version);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "install") {
                std::string server, version;
                iss >> server >> version;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Installing MCP server: " << server << std::endl;
                std::string result = server_wrapper_->installMCPServer(server, version);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "uninstall") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Uninstalling MCP server: " << server << std::endl;
                std::string result = server_wrapper_->uninstallMCPServer(server);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "update") {
                std::string server, version;
                iss >> server >> version;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Updating MCP server: " << server << std::endl;
                std::string result = server_wrapper_->updateMCPServer(server, version);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "list") {
                std::cout << "Listing installed servers..." << std::endl;
                std::string result = server_wrapper_->listInstalledServers();
                std::cout << "Installed servers: " << result << std::endl;

            } else if (cmd == "search") {
                std::string query;
                iss >> query;
                if (query.empty()) query = "MCP server";
                std::cout << "Searching for MCP servers: " << query << std::endl;
                std::string result = server_wrapper_->searchMCPServers(query);
                std::cout << "Search results: " << result << std::endl;

            } else if (cmd == "start") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Starting MCP server: " << server << std::endl;
                std::string result = server_wrapper_->startMCPServer(server);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "stop") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Stopping MCP server: " << server << std::endl;
                std::string result = server_wrapper_->stopMCPServer(server);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "restart") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Restarting MCP server: " << server << std::endl;
                std::string result = server_wrapper_->restartMCPServer(server);
                std::cout << "Result: " << result << std::endl;

            } else if (cmd == "status") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Getting all server status..." << std::endl;
                    std::string result = server_wrapper_->getAllServerStatus();
                    std::cout << "Server status: " << result << std::endl;
                } else {
                    std::cout << "Getting server status: " << server << std::endl;
                    std::string result = server_wrapper_->getServerStatus(server);
                    std::cout << "Server status: " << result << std::endl;
                }

            } else if (cmd == "progress") {
                std::string jobId;
                iss >> jobId;
                if (jobId.empty()) {
                    std::cout << "Error: Job ID required" << std::endl;
                    return true;
                }
                std::cout << "Getting download progress: " << jobId << std::endl;
                std::string result = server_wrapper_->getDownloadProgress(jobId);
                std::cout << "Progress: " << result << std::endl;

            } else if (cmd == "logs") {
                std::string server;
                int lines = 100;
                iss >> server >> lines;
                if (server.empty()) {
                    std::cout << "Error: Server name required" << std::endl;
                    return true;
                }
                std::cout << "Getting server logs: " << server << " (lines: " << lines << ")" << std::endl;
                std::string result = server_wrapper_->getServerLogs(server, lines);
                std::cout << "Server logs: " << result << std::endl;

            } else if (cmd == "config") {
                std::string server;
                iss >> server;
                if (server.empty()) {
                    std::cout << "Getting all configurations..." << std::endl;
                    std::string result = server_wrapper_->getConfiguration();
                    std::cout << "Configurations: " << result << std::endl;
                } else {
                    std::cout << "Getting server configuration: " << server << std::endl;
                    std::string result = server_wrapper_->getConfiguration(server);
                    std::cout << "Configuration: " << result << std::endl;
                }

            } else if (!cmd.empty()) {
                std::cout << "Unknown command: " << cmd << std::endl;
                std::cout << "Type 'quit' to exit or see menu above for available commands." << std::endl;
            }

        } catch (const std::exception& e) {
            std::cout << "Error executing command: " << e.what() << std::endl;
        }

        return true;
    }
};

int main() {
    try {
        MCPServerManagerCLI cli;
        cli.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}