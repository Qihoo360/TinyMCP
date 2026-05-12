#include "mcp/prompts.hpp"
#include "mcp/utils/logger.hpp"
#include <stdexcept>
#include <sstream>

namespace mcp {

// PromptRegistry implementation
void PromptRegistry::registerPrompt(const std::string& name, const PromptInfo& info, std::function<nlohmann::json(const nlohmann::json&)> handler) {
    if (name.empty()) {
        throw std::invalid_argument("Prompt name cannot be empty");
    }
    
    if (prompts_.find(name) != prompts_.end()) {
        utils::getLogger().warn("Overwriting existing prompt: " + name);
    }
    
    prompts_[name] = {info, handler};
    utils::getLogger().info("Registered prompt: " + name);
}

void PromptRegistry::unregisterPrompt(const std::string& name) {
    auto it = prompts_.find(name);
    if (it != prompts_.end()) {
        prompts_.erase(it);
        utils::getLogger().info("Unregistered prompt: " + name);
    }
}

nlohmann::json PromptRegistry::getPrompt(const std::string& name, const nlohmann::json& arguments) {
    auto it = prompts_.find(name);
    if (it == prompts_.end()) {
        throw MCPError(ErrorCode::MethodNotFound, "Prompt not found: " + name);
    }
    
    try {
        return it->second.handler(arguments);
    } catch (const std::exception& e) {
        throw MCPError(ErrorCode::InternalError, "Prompt execution error: " + std::string(e.what()));
    }
}

std::map<std::string, PromptInfo> PromptRegistry::getPrompts() const {
    std::map<std::string, PromptInfo> result;
    for (const auto& [name, prompt] : prompts_) {
        result[name] = prompt.info;
    }
    return result;
}

bool PromptRegistry::hasPrompt(const std::string& name) const {
    return prompts_.find(name) != prompts_.end();
}

// Example prompts implementation
nlohmann::json CodeGenerationPrompt::generate(const nlohmann::json& arguments) {
    if (!arguments.contains("language")) {
        throw std::invalid_argument("Missing required parameter: language");
    }
    
    if (!arguments.contains("description")) {
        throw std::invalid_argument("Missing required parameter: description");
    }
    
    std::string language = arguments["language"].get<std::string>();
    std::string description = arguments["description"].get<std::string>();
    std::string style = arguments.contains("style") ? arguments["style"].get<std::string>() : "clean";
    std::string comments = arguments.contains("comments") ? arguments["comments"].get<std::string>() : "detailed";
    
    std::ostringstream prompt;
    prompt << "Generate " << language << " code for the following description:\n\n";
    prompt << "Description: " << description << "\n\n";
    prompt << "Requirements:\n";
    prompt << "- Use " << style << " coding style\n";
    prompt << "- Include " << comments << " comments\n";
    prompt << "- Follow best practices for " << language << "\n";
    prompt << "- Ensure the code is production-ready\n\n";
    prompt << "Please provide the complete code implementation.";
    
    nlohmann::json result;
    result["description"] = "Code generation prompt for " + language;
    result["messages"] = nlohmann::json::array();
    
    nlohmann::json message;
    message["role"] = "user";
    message["content"] = {
        {"type", "text"},
        {"text", prompt.str()}
    };
    result["messages"].push_back(message);
    
    return result;
}

PromptInfo CodeGenerationPrompt::getInfo() const {
    PromptInfo info;
    info.name = "code_generation";
    info.description = "Generate code based on a description";
    info.arguments = {
        {"type", "object"},
        {"properties", {
            {"language", {
                {"type", "string"},
                {"description", "Programming language for code generation"}
            }},
            {"description", {
                {"type", "string"},
                {"description", "Description of what the code should do"}
            }},
            {"style", {
                {"type", "string"},
                {"enum", {"clean", "verbose", "minimal"}},
                {"description", "Code style preference"},
                {"default", "clean"}
            }},
            {"comments", {
                {"type", "string"},
                {"enum", {"detailed", "minimal", "none"}},
                {"description", "Comment level preference"},
                {"default", "detailed"}
            }}
        }},
        {"required", nlohmann::json::array({"language", "description"})}
    };
    return info;
}

nlohmann::json DocumentationPrompt::generate(const nlohmann::json& arguments) {
    if (!arguments.contains("topic")) {
        throw std::invalid_argument("Missing required parameter: topic");
    }
    
    std::string topic = arguments["topic"].get<std::string>();
    std::string format = arguments.contains("format") ? arguments["format"].get<std::string>() : "markdown";
    std::string audience = arguments.contains("audience") ? arguments["audience"].get<std::string>() : "developers";
    std::string level = arguments.contains("level") ? arguments["level"].get<std::string>() : "intermediate";
    
    std::ostringstream prompt;
    prompt << "Create comprehensive documentation for: " << topic << "\n\n";
    prompt << "Format: " << format << "\n";
    prompt << "Target audience: " << audience << "\n";
    prompt << "Technical level: " << level << "\n\n";
    prompt << "The documentation should include:\n";
    prompt << "- Clear introduction and overview\n";
    prompt << "- Detailed explanations with examples\n";
    prompt << "- Best practices and common pitfalls\n";
    prompt << "- References and further reading\n\n";
    prompt << "Please provide well-structured documentation.";
    
    nlohmann::json result;
    result["description"] = "Documentation generation for " + topic;
    result["messages"] = nlohmann::json::array();
    
    nlohmann::json message;
    message["role"] = "user";
    message["content"] = {
        {"type", "text"},
        {"text", prompt.str()}
    };
    result["messages"].push_back(message);
    
    return result;
}

PromptInfo DocumentationPrompt::getInfo() const {
    PromptInfo info;
    info.name = "documentation";
    info.description = "Generate documentation for a given topic";
    info.arguments = {
        {"type", "object"},
        {"properties", {
            {"topic", {
                {"type", "string"},
                {"description", "Topic to document"}
            }},
            {"format", {
                {"type", "string"},
                {"enum", {"markdown", "html", "plain"}},
                {"description", "Documentation format"},
                {"default", "markdown"}
            }},
            {"audience", {
                {"type", "string"},
                {"enum", {"beginners", "developers", "experts"}},
                {"description", "Target audience"},
                {"default", "developers"}
            }},
            {"level", {
                {"type", "string"},
                {"enum", {"basic", "intermediate", "advanced"}},
                {"description", "Technical complexity level"},
                {"default", "intermediate"}
            }}
        }},
        {"required", nlohmann::json::array({"topic"})}
    };
    return info;
}

nlohmann::json DebuggingPrompt::generate(const nlohmann::json& arguments) {
    if (!arguments.contains("code")) {
        throw std::invalid_argument("Missing required parameter: code");
    }
    
    if (!arguments.contains("error")) {
        throw std::invalid_argument("Missing required parameter: error");
    }
    
    std::string code = arguments["code"].get<std::string>();
    std::string error = arguments["error"].get<std::string>();
    std::string language = arguments.contains("language") ? arguments["language"].get<std::string>() : "unknown";
    std::string context = arguments.contains("context") ? arguments["context"].get<std::string>() : "No additional context";
    
    std::ostringstream prompt;
    prompt << "Help debug the following " << language << " code:\n\n";
    prompt << "Code:\n```" << language << "\n" << code << "\n```\n\n";
    prompt << "Error:\n" << error << "\n\n";
    prompt << "Context:\n" << context << "\n\n";
    prompt << "Please provide:\n";
    prompt << "1. Analysis of the error\n";
    prompt << "2. Specific fix for the code\n";
    prompt << "3. Explanation of why the error occurred\n";
    prompt << "4. Suggestions to prevent similar issues\n\n";
    prompt << "Provide a corrected version of the code.";
    
    nlohmann::json result;
    result["description"] = "Debugging assistance for " + language + " code";
    result["messages"] = nlohmann::json::array();
    
    nlohmann::json message;
    message["role"] = "user";
    message["content"] = {
        {"type", "text"},
        {"text", prompt.str()}
    };
    result["messages"].push_back(message);
    
    return result;
}

PromptInfo DebuggingPrompt::getInfo() const {
    PromptInfo info;
    info.name = "debugging";
    info.description = "Get help debugging code with error messages";
    info.arguments = {
        {"type", "object"},
        {"properties", {
            {"code", {
                {"type", "string"},
                {"description", "The code that has an error"}
            }},
            {"error", {
                {"type", "string"},
                {"description", "The error message or description"}
            }},
            {"language", {
                {"type", "string"},
                {"description", "Programming language of the code"},
                {"default", "unknown"}
            }},
            {"context", {
                {"type", "string"},
                {"description", "Additional context about the error"},
                {"default", "No additional context"}
            }}
        }},
        {"required", nlohmann::json::array({"code", "error"})}
    };
    return info;
}

} // namespace mcp