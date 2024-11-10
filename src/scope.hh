#pragma once
#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Custom exception classes for better error categorization
class ScopeError : public std::runtime_error
{
public:
    explicit ScopeError(const std::string &msg)
        : std::runtime_error("Scope Error: " + msg)
    {}
};

class SymbolError : public std::runtime_error
{
public:
    explicit SymbolError(const std::string &msg)
        : std::runtime_error("Symbol Error: " + msg)
    {}
};

template<typename T>
class ScopeManager
{
public:
    // Define a type alias for scope identification
    using ScopeId = size_t;

    ScopeManager()
    {
        // Initialize the global scope explicitly
        scopes_.clear();
        auto globalScope = std::make_shared<std::unordered_map<std::string, T>>();
        if (!globalScope) {
            throw ScopeError("Failed to create global scope");
        }
        scopes_.push_back(globalScope);
        currentScopeStack_.push(0); // Global scope has ID 0

        if (scopes_.empty() || !scopes_[0]) {
            throw ScopeError("Failed to initialize ScopeManager with global scope");
        }
    }

    // Enter a new scope and return its ID
    ScopeId enterScope()
    {
        scopes_.push_back(std::make_shared<std::unordered_map<std::string, T>>());
        ScopeId newScopeId = scopes_.size() - 1;
        currentScopeStack_.push(newScopeId);
        return newScopeId;
    }

    // Enter an existing scope by ID
    void enterExistingScope(ScopeId scopeId)
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }
        currentScopeStack_.push(scopeId);
    }

    // Exit the current scope
    void exitScope()
    {
        if (currentScopeStack_.size() <= 1) {
            throw ScopeError("Cannot exit global scope");
        }
        currentScopeStack_.pop();
    }

    // Add an item to the current scope with optional shadowing check
    void add(const std::string &name, const T &item, bool allowShadowing = false)
    {
        if (currentScope().count(name) > 0) {
            throw SymbolError("Item already exists in the current scope: " + name);
        }
        // Optionally prevent shadowing of symbols from outer scopes
        if (!allowShadowing && isVisibleInCurrentScope(name)) {
            throw SymbolError("Shadowing not allowed: symbol '" + name
                              + "' is already visible in outer scope");
        }
        currentScope()[name] = item;
    }

    // Add an item to a specific scope
    void addToScope(ScopeId scopeId,
                    const std::string &name,
                    const T &item,
                    bool allowShadowing = false)
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }

        auto &targetScope = *scopes_[scopeId];
        if (targetScope.count(name) > 0) {
            throw SymbolError("Item already exists in the target scope: " + name);
        }

        if (!allowShadowing) {
            // Check for shadowing in outer scopes
            bool shadows = false;
            for (size_t i = 0; i < scopeId; ++i) {
                if (scopes_[i]->count(name) > 0) {
                    shadows = true;
                    break;
                }
            }
            if (shadows) {
                throw SymbolError("Shadowing not allowed: symbol '" + name
                                  + "' exists in outer scope");
            }
        }

        targetScope[name] = item;
    }

    // Add an item to the global scope
    void addGlobal(const std::string &name, const T &item)
    {
        if (globalScope().count(name) > 0) {
            throw SymbolError("Item already exists in global scope: " + name);
        }
        globalScope()[name] = item;
    }

    // Get an item from the nearest scope where it exists, without creating new shared_ptrs unnecessarily
    const T *get(const std::string &name) const
    {
        std::stack<ScopeId> tempStack = currentScopeStack_;
        while (!tempStack.empty()) {
            ScopeId currentId = tempStack.top();
            auto found = scopes_[currentId]->find(name);
            if (found != scopes_[currentId]->end()) {
                return &found->second;
            }
            tempStack.pop();
        }
        return nullptr;
    }

    // Get an item from a specific scope
    const T *getFromScope(ScopeId scopeId, const std::string &name) const
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }

        auto found = scopes_[scopeId]->find(name);
        if (found != scopes_[scopeId]->end()) {
            return &found->second;
        }
        return nullptr;
    }

    // Check if an item exists in any scope
    bool exists(const std::string &name) const
    {
        std::stack<ScopeId> tempStack = currentScopeStack_;
        while (!tempStack.empty()) {
            if (scopes_[tempStack.top()]->count(name) > 0) {
                return true;
            }
            tempStack.pop();
        }
        return false;
    }

    // Check if an item exists in a specific scope
    bool existsInScope(ScopeId scopeId, const std::string &name) const
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }
        return scopes_[scopeId]->count(name) > 0;
    }

    // Check if an item exists in the current scope only
    bool existsInCurrentScope(const std::string &name) const
    {
        return scopes_[currentScopeStack_.top()]->count(name) > 0;
    }

    // Check if an item is visible in any current or outer scope
    bool isVisibleInCurrentScope(const std::string &name) const { return exists(name); }

    // Update an item in the nearest scope where it exists
    bool update(const std::string &name, const T &newItem)
    {
        std::stack<ScopeId> tempStack = currentScopeStack_;
        while (!tempStack.empty()) {
            auto &scope = *scopes_[tempStack.top()];
            auto found = scope.find(name);
            if (found != scope.end()) {
                found->second = newItem;
                return true;
            }
            tempStack.pop();
        }
        return false;
    }

    // Update an item in a specific scope
    bool updateInScope(ScopeId scopeId, const std::string &name, const T &newItem)
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }

        auto &scope = *scopes_[scopeId];
        auto found = scope.find(name);
        if (found != scope.end()) {
            found->second = newItem;
            return true;
        }
        return false;
    }

    // Get the current scope depth
    size_t getCurrentScopeDepth() const { return currentScopeStack_.size() - 1; }

    // Get the current scope ID
    ScopeId getCurrentScopeId() const { return currentScopeStack_.top(); }

    // Retrieve a copy of a specific scope
    std::unordered_map<std::string, T> getScopeCopy(ScopeId scopeId) const
    {
        if (scopeId >= scopes_.size()) {
            throw ScopeError("Invalid scope ID: " + std::to_string(scopeId));
        }
        return *scopes_[scopeId];
    }

    // Retrieve a copy of the current scope
    std::unordered_map<std::string, T> getCurrentScope() const
    {
        return *scopes_[currentScopeStack_.top()];
    }

    // Retrieve all symbols visible in the current context (all scopes up to the current)
    std::unordered_map<std::string, T> getAllVisibleSymbols() const
    {
        std::unordered_map<std::string, T> visibleSymbols;
        std::stack<ScopeId> tempStack = currentScopeStack_;
        while (!tempStack.empty()) {
            for (const auto &pair : *scopes_[tempStack.top()]) {
                if (visibleSymbols.find(pair.first) == visibleSymbols.end()) {
                    visibleSymbols[pair.first] = pair.second;
                }
            }
            tempStack.pop();
        }
        return visibleSymbols;
    }

private:
    std::vector<std::shared_ptr<std::unordered_map<std::string, T>>> scopes_;
    std::stack<ScopeId> currentScopeStack_; // Stack to track the current scope hierarchy

    // Access the current scope
    std::unordered_map<std::string, T> &currentScope()
    {
        return *scopes_[currentScopeStack_.top()];
    }

    const std::unordered_map<std::string, T> &currentScope() const
    {
        return *scopes_[currentScopeStack_.top()];
    }

    // Access the global scope
    std::unordered_map<std::string, T> &globalScope() { return *scopes_.front(); }

    const std::unordered_map<std::string, T> &globalScope() const { return *scopes_.front(); }
};
