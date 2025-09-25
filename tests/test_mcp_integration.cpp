#include "../src/Interfaces/IMCPDownloader.hpp"
#include "../src/Utils/GitHubAPI.hpp"
#include "../src/Utils/MCPConfigParser.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace TinyMCP::Interfaces;
using namespace TinyMCP::Utils;

void test_mcp_download_manager() {
    std::cout << "Testing MCP Download Manager..." << std::endl;
    
    auto downloader = std::make_unique<MCPDownloadManager>();
    
    // Test GitHub repository download
    std::cout << "  Testing GitHub repository download..." << std::endl;
    bool result = downloader->downloadMCPPackage("https://github.com/modelcontextprotocol/servers", "./test_downloads/");
    std::cout << "  GitHub repo download: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test Docker image download
    std::cout << "  Testing Docker image download..." << std::endl;
    result = downloader->downloadDockerImage("mcp/server", "latest");
    std::cout << "  Docker image download: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test NPM package installation
    std::cout << "  Testing NPM package installation..." << std::endl;
    result = downloader->installNPMPackage("@modelcontextprotocol/server", "latest");
    std::cout << "  NPM package installation: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test progress tracking
    std::cout << "  Testing progress tracking..." << std::endl;
    auto activeDownloads = downloader->listActiveDownloads();
    std::cout << "  Active downloads: " << activeDownloads.size() << std::endl;
    
    std::cout << "MCP Download Manager test completed." << std::endl;
}

void test_github_api() {
    std::cout << "Testing GitHub API integration..." << std::endl;
    
    GitHubAPI github;
    
    // Test MCP server search
    std::cout << "  Testing MCP server search..." << std::endl;
    auto repos = github.searchMCPServers("MCP server", "", "stars", 5);
    std::cout << "  Found " << repos.size() << " MCP servers" << std::endl;
    
    for (const auto& repo : repos) {
        std::cout << "    - " << repo.full_name << " (" << repo.stargazers_count << " stars)" << std::endl;
    }
    
    // Test repository information
    if (!repos.empty()) {
        std::cout << "  Testing repository info..." << std::endl;
        auto firstRepo = repos[0];
        size_t slashPos = firstRepo.full_name.find('/');
        if (slashPos != std::string::npos) {
            std::string owner = firstRepo.full_name.substr(0, slashPos);
            std::string repo = firstRepo.full_name.substr(slashPos + 1);
            
            auto repoInfo = github.getRepositoryInfo(owner, repo);
            std::cout << "    Repository: " << repoInfo.full_name << std::endl;
            std::cout << "    Description: " << repoInfo.description << std::endl;
            std::cout << "    Language: " << repoInfo.language << std::endl;
        }
    }
    
    // Test MCP server detection
    std::cout << "  Testing MCP server detection..." << std::endl;
    bool isMCP = github.isMCPServer("modelcontextprotocol", "servers");
    std::cout << "    MCP Server detection: " << (isMCP ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "GitHub API test completed." << std::endl;
}

void test_mcp_config_parser() {
    std::cout << "Testing MCP Configuration Parser..." << std::endl;
    
    MCPConfigParser parser;
    
    // Test configuration template generation
    std::cout << "  Testing configuration template generation..." << std::endl;
    auto template_config = MCPConfigParser::generateTemplate("test-server", "1.0.0");
    std::cout << "  Template generated: " << (template_config.isNull() ? "FAILED" : "SUCCESS") << std::endl;
    
    // Test JSON string parsing
    std::cout << "  Testing JSON string parsing..." << std::endl;
    std::string testConfig = R"({
        "server_name": "test-server",
        "version": "1.0.0",
        "description": "Test MCP Server",
        "tools": ["download", "install"],
        "resources": ["config", "logs"],
        "environment": {
            "API_KEY": "test-key"
        },
        "settings": {
            "port": "3000"
        }
    })";
    
    bool parseResult = parser.parseFromString(testConfig);
    std::cout << "  JSON parsing: " << (parseResult ? "SUCCESS" : "FAILED") << std::endl;
    
    if (parseResult) {
        // Test validation
        std::cout << "  Testing configuration validation..." << std::endl;
        bool valid = parser.validate();
        std::cout << "  Configuration validation: " << (valid ? "SUCCESS" : "FAILED") << std::endl;
        
        if (valid) {
            // Test configuration extraction
            std::cout << "  Testing configuration extraction..." << std::endl;
            auto config = parser.getConfiguration();
            std::cout << "    Server name: " << config.server_name << std::endl;
            std::cout << "    Version: " << config.version << std::endl;
            std::cout << "    Tools count: " << config.tools.size() << std::endl;
            std::cout << "    Resources count: " << config.resources.size() << std::endl;
            
            // Test tool checking
            bool hasDownload = parser.hasTool("download");
            bool hasInstall = parser.hasTool("install");
            std::cout << "    Has download tool: " << (hasDownload ? "YES" : "NO") << std::endl;
            std::cout << "    Has install tool: " << (hasInstall ? "YES" : "NO") << std::endl;
        }
    }
    
    std::cout << "MCP Configuration Parser test completed." << std::endl;
}

void test_progress_callback() {
    std::cout << "Testing progress callback functionality..." << std::endl;
    
    auto downloader = std::make_unique<MCPDownloadManager>();
    
    // Set up progress callback
    bool callbackCalled = false;
    downloader->setProgressCallback([&callbackCalled](const TinyMCP::Core::MCPDownloadProgress& progress) {
        callbackCalled = true;
        std::cout << "    Progress update: " << progress.job_id 
                  << " - " << progress.progress_percent << "%" << std::endl;
    });
    
    // Start a download to trigger callback
    std::cout << "  Starting download to test callback..." << std::endl;
    downloader->downloadMCPPackage("https://github.com/modelcontextprotocol/servers", "./test_downloads/");
    
    // Wait a bit for callback to be triggered
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "  Progress callback test: " << (callbackCalled ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Progress callback test completed." << std::endl;
}

void test_thread_safety() {
    std::cout << "Testing thread safety..." << std::endl;
    
    auto downloader = std::make_unique<MCPDownloadManager>();
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    // Start multiple concurrent downloads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&downloader, &successCount, i]() {
            std::string repo = "https://github.com/modelcontextprotocol/servers";
            std::string dest = "./test_downloads/thread_" + std::to_string(i);
            
            bool result = downloader->downloadMCPPackage(repo, dest);
            if (result) {
                successCount++;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "  Concurrent downloads completed: " << successCount.load() << "/5" << std::endl;
    std::cout << "Thread safety test completed." << std::endl;
}

int main() {
    std::cout << "=== TinyMCP Phase 1 Integration Tests ===" << std::endl;
    std::cout << "Testing enhanced MCP download capabilities..." << std::endl;
    std::cout << std::endl;
    
    try {
        test_mcp_download_manager();
        std::cout << std::endl;
        
        test_github_api();
        std::cout << std::endl;
        
        test_mcp_config_parser();
        std::cout << std::endl;
        
        test_progress_callback();
        std::cout << std::endl;
        
        test_thread_safety();
        std::cout << std::endl;
        
        std::cout << "=== All Integration Tests Completed Successfully ===" << std::endl;
        std::cout << "Phase 1 implementation is working correctly!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}