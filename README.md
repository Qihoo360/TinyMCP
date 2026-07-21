# TinyMCP Enhanced

## Enhanced Features

This fork adds significant download capabilities to the original TinyMCP SDK:

### 🚀 New Features

- **Resumable Downloads**: Downloads can be paused and resumed using session IDs
- **Thread-Safe Architecture**: Thread-safe queues and priority handling for robust operation  
- **Chunk-Based Processing**: Downloads process in chunks for better resource management
- **Session Persistence**: Download state is saved and can be restored after application restart
- **CLI Interface**: Simple command-line interface for testing download functionality
- **Progress Tracking**: Real-time download progress with callbacks
- **HTTP Client Integration**: Built-in libcurl integration for reliable downloads

### 🔧 Technical Enhancements

- **ThreadSafeQueue**: Lock-free queue implementation for message passing
- **ThreadSafePriorityQueue**: Priority-based message handling (quit=0, download=1, etc.)
- **HttpClient**: Robust HTTP client with resume capability using libcurl
- **SessionPersistence**: JSON-based session storage for download state
- **DownloadTask**: MCP Task implementation for download operations

### 📋 Architecture

```
TinyMCP Enhanced
├── Utils/
│   ├── ThreadSafeQueue.hpp      # Thread-safe message queues
│   ├── HttpClient.hpp           # HTTP download client
│   └── SessionPersistence.hpp   # Session state management
├── Task/
│   └── DownloadTask.hpp         # Download task implementation
└── Examples/
    └── DownloadCLI.cpp         # CLI demonstration
```

### 🎯 Usage

```bash
# Build
mkdir build && cd build
cmake ..
make

# Run CLI
./download_cli

# Commands:
download <URL> [session_id]  # Start/resume download
list                         # List active sessions  
quit                         # Exit
```

### 🧪 Examples

```bash
# Start new download
download https://example.com/largefile.zip

# Resume download with session ID
download https://example.com/largefile.zip abc123ef

# List all sessions
list
```

### 🔧 Dependencies

- jsoncpp 1.9.5 (original)
- libcurl (new)
- C++20 compiler (Clang recommended)

### 🎁 Integration with MCP

All enhancements maintain full compatibility with the MCP specification and can be exposed as MCP Tools:

- `download_file`: Tool for initiating downloads
- `resume_download`: Tool for resuming existing sessions
- `list_downloads`: Tool for listing download status

## Original TinyMCP Features

TinyMCP is a lightweight C++ SDK for implementing the MCP Server.

[Original README content continues...]
