#pragma once

#include "types.hpp"
#include "message.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace mcp {

/**
 * @brief Prompt handler interface
 */
class PromptHandler {
public:
    virtual ~PromptHandler() = default;
    
    /**
     * @brief Handle prompt list request
     */
    virtual ListPromptsResult listPrompts() = 0;
    
    /**
     * @brief Handle get prompt request
     */
    virtual GetPromptResult getPrompt(const GetPromptParams& params) = 0;
};

/**
 * @brief Default prompt handler implementation
 */
class DefaultPromptHandler : public PromptHandler {
private:
    std::unordered_map<std::string, Prompt> prompts_;
    std::unordered_map<std::string, std::function<GetPromptResult(const GetPromptParams&)>> prompt_handlers_;
    std::mutex prompts_mutex_;

public:
    /**
     * @brief Register a prompt
     */
    void registerPrompt(const Prompt& prompt, std::function<GetPromptResult(const GetPromptParams&)> handler);
    
    /**
     * @brief Unregister a prompt
     */
    void unregisterPrompt(const std::string& name);
    
    /**
     * @brief Check if prompt is registered
     */
    bool hasPrompt(const std::string& name) const;
    
    /**
     * @brief Get registered prompts
     */
    std::vector<Prompt> getPrompts() const;
    
    // PromptHandler interface
    ListPromptsResult listPrompts() override;
    GetPromptResult getPrompt(const GetPromptParams& params) override;
};

/**
 * @brief Prompt registry for managing prompts
 */
class PromptRegistry {
private:
    std::unordered_map<std::string, Prompt> prompts_;
    std::unordered_map<std::string, std::function<GetPromptResult(const GetPromptParams&)>> handlers_;
    std::mutex registry_mutex_;

public:
    /**
     * @brief Register a prompt
     */
    void registerPrompt(const Prompt& prompt, std::function<GetPromptResult(const GetPromptParams&)> handler);
    
    /**
     * @brief Unregister a prompt
     */
    void unregisterPrompt(const std::string& name);
    
    /**
     * @brief Check if prompt is registered
     */
    bool hasPrompt(const std::string& name) const;
    
    /**
     * @brief Get prompt by name
     */
    std::optional<Prompt> getPrompt(const std::string& name) const;
    
    /**
     * @brief Get all registered prompts
     */
    std::vector<Prompt> getAllPrompts() const;
    
    /**
     * @brief Get prompt handler
     */
    std::optional<std::function<GetPromptResult(const GetPromptParams&)>> getHandler(const std::string& name) const;
    
    /**
     * @brief Clear all prompts
     */
    void clear();
    
    /**
     * @brief Get prompt count
     */
    size_t size() const;
};

/**
 * @brief Built-in prompts
 */
namespace builtin {

/**
 * @brief Code review prompt
 */
GetPromptResult codeReviewPrompt(const GetPromptParams& params);

/**
 * @brief Bug fix prompt
 */
GetPromptResult bugFixPrompt(const GetPromptParams& params);

/**
 * @brief Documentation prompt
 */
GetPromptResult documentationPrompt(const GetPromptParams& params);

/**
 * @brief Test generation prompt
 */
GetPromptResult testGenerationPrompt(const GetPromptParams& params);

/**
 * @brief Refactoring prompt
 */
GetPromptResult refactoringPrompt(const GetPromptParams& params);

/**
 * @brief Performance optimization prompt
 */
GetPromptResult performanceOptimizationPrompt(const GetPromptParams& params);

/**
 * @brief Security audit prompt
 */
GetPromptResult securityAuditPrompt(const GetPromptParams& params);

/**
 * @brief API documentation prompt
 */
GetPromptResult apiDocumentationPrompt(const GetPromptParams& params);

/**
 * @brief Error handling prompt
 */
GetPromptResult errorHandlingPrompt(const GetPromptParams& params);

/**
 * @brief Code explanation prompt
 */
GetPromptResult codeExplanationPrompt(const GetPromptParams& params);

/**
 * @brief Register all built-in prompts
 */
void registerBuiltinPrompts(PromptRegistry& registry);

} // namespace builtin

/**
 * @brief Prompt template engine
 */
class PromptTemplateEngine {
public:
    /**
     * @brief Render prompt template
     */
    static std::string renderTemplate(const std::string& template_str, 
                                    const std::unordered_map<std::string, std::string>& variables);
    
    /**
     * @brief Validate prompt template
     */
    static bool validateTemplate(const std::string& template_str);
    
    /**
     * @brief Extract variables from template
     */
    static std::vector<std::string> extractVariables(const std::string& template_str);
    
    /**
     * @brief Check if template has required variables
     */
    static bool hasRequiredVariables(const std::string& template_str, 
                                   const std::unordered_map<std::string, std::string>& variables);
};

/**
 * @brief Prompt parameter validation
 */
class PromptParameterValidator {
public:
    /**
     * @brief Validate prompt parameters
     */
    static bool validateParameters(const Prompt& prompt, const std::optional<std::unordered_map<std::string, JsonValue>>& arguments);
    
    /**
     * @brief Validate required parameters
     */
    static bool validateRequiredParameters(const std::vector<PromptArgument>& arguments,
                                          const std::unordered_map<std::string, JsonValue>& values);
    
    /**
     * @brief Get validation error message
     */
    static std::string getValidationError(const Prompt& prompt, 
                                        const std::optional<std::unordered_map<std::string, JsonValue>>& arguments);
};

/**
 * @brief Prompt context
 */
struct PromptContext {
    std::string prompt_name;
    std::unordered_map<std::string, JsonValue> arguments;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief Prompt execution result
 */
struct PromptExecutionResult {
    bool success;
    std::vector<Content> description;
    std::vector<Content> messages;
    std::optional<std::string> error_message;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
    std::optional<std::chrono::milliseconds> execution_time;
};

/**
 * @brief Prompt executor interface
 */
class PromptExecutor {
public:
    virtual ~PromptExecutor() = default;
    
    /**
     * @brief Execute a prompt
     */
    virtual PromptExecutionResult execute(const PromptContext& context) = 0;
    
    /**
     * @brief Check if prompt can be executed
     */
    virtual bool canExecute(const std::string& promptName) = 0;
    
    /**
     * @brief Get prompt execution time limit
     */
    virtual std::chrono::milliseconds getExecutionTimeLimit() const = 0;
};

/**
 * @brief Default prompt executor implementation
 */
class DefaultPromptExecutor : public PromptExecutor {
private:
    PromptRegistry& registry_;
    std::chrono::milliseconds time_limit_;

public:
    DefaultPromptExecutor(PromptRegistry& registry, std::chrono::milliseconds timeLimit = std::chrono::seconds(30));
    
    PromptExecutionResult execute(const PromptContext& context) override;
    bool canExecute(const std::string& promptName) override;
    std::chrono::milliseconds getExecutionTimeLimit() const override;
    
    /**
     * @brief Set execution time limit
     */
    void setExecutionTimeLimit(std::chrono::milliseconds limit);
};

/**
 * @brief Prompt manager for high-level prompt operations
 */
class PromptManager {
private:
    PromptRegistry registry_;
    std::unique_ptr<PromptExecutor> executor_;
    std::unique_ptr<PromptTemplateEngine> template_engine_;

public:
    PromptManager();
    
    /**
     * @brief Register a prompt
     */
    void registerPrompt(const Prompt& prompt, std::function<GetPromptResult(const GetPromptParams&)> handler);
    
    /**
     * @brief Unregister a prompt
     */
    void unregisterPrompt(const std::string& name);
    
    /**
     * @brief List all prompts
     */
    ListPromptsResult listPrompts();
    
    /**
     * @brief Get a prompt
     */
    GetPromptResult getPrompt(const GetPromptParams& params);
    
    /**
     * @brief Check if prompt exists
     */
    bool hasPrompt(const std::string& name) const;
    
    /**
     * @brief Get prompt info
     */
    std::optional<Prompt> getPromptInfo(const std::string& name) const;
    
    /**
     * @brief Set executor
     */
    void setExecutor(std::unique_ptr<PromptExecutor> executor);
    
    /**
     * @brief Set template engine
     */
    void setTemplateEngine(std::unique_ptr<PromptTemplateEngine> template_engine);
};

/**
 * @brief Prompt categories
 */
namespace prompt_categories {

inline constexpr const char* CODE_REVIEW = "code_review";
inline constexpr const char* BUG_FIX = "bug_fix";
inline constexpr const char* DOCUMENTATION = "documentation";
inline constexpr const char* TESTING = "testing";
inline constexpr const char* REFACTORING = "refactoring";
inline constexpr const char* PERFORMANCE = "performance";
inline constexpr const char* SECURITY = "security";
inline constexpr const char* API_DOCS = "api_docs";
inline constexpr const char* ERROR_HANDLING = "error_handling";
inline constexpr const char* EXPLANATION = "explanation";
inline constexpr const char* GENERAL = "general";

} // namespace prompt_categories

/**
 * @brief Prompt difficulty levels
 */
namespace prompt_difficulty {

inline constexpr const char* BEGINNER = "beginner";
inline constexpr const char* INTERMEDIATE = "intermediate";
inline constexpr const char* ADVANCED = "advanced";
inline constexpr const char* EXPERT = "expert";

} // namespace prompt_difficulty

} // namespace mcp