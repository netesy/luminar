#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
class ScopeManager
{
public:
    ScopeManager()
    {
        // Initialize with a global scope
        enterScope();
    }

    // Enter a new scope
    void enterScope() { scopes_.push_back(std::make_shared<std::unordered_map<std::string, T>>()); }

    // Exit the current scope
    void exitScope()
    {
        if (scopes_.size() <= 1) {
            throw std::runtime_error("Cannot exit global scope");
        }
        scopes_.pop_back();
    }

    // Add an item to the current scope
    void add(const std::string &name, const T &item)
    {
        if (currentScope().count(name) > 0) {
            throw std::runtime_error("Item already exists in current scope: " + name);
        }
        currentScope()[name] = item;
    }

    // Add an item to the global scope
    void addGlobal(const std::string &name, const T &item)
    {
        if (globalScope().count(name) > 0) {
            throw std::runtime_error("Item already exists in global scope: " + name);
        }
        globalScope()[name] = item;
    }

    // Get an item from the nearest scope
    std::shared_ptr<T> get(const std::string &name) const
    {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = (*it)->find(name);
            if (found != (*it)->end()) {
                return std::make_shared<T>(found->second);
            }
        }
        return nullptr;
    }

    // Check if an item exists in any scope
    bool exists(const std::string &name) const
    {
        for (const auto &scope : scopes_) {
            if (scope->count(name) > 0) {
                return true;
            }
        }
        return false;
    }

    // Update an item in the nearest scope where it exists
    bool update(const std::string &name, const T &newItem)
    {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = (*it)->find(name);
            if (found != (*it)->end()) {
                found->second = newItem;
                return true;
            }
        }
        return false;
    }

    // Get the current scope depth
    size_t getCurrentScopeDepth() const
    {
        return scopes_.size() - 1; // Subtract 1 to account for global scope
    }

private:
    std::vector<std::shared_ptr<std::unordered_map<std::string, T>>> scopes_;

    std::unordered_map<std::string, T> &currentScope() { return *scopes_.back(); }

    const std::unordered_map<std::string, T> &currentScope() const { return *scopes_.back(); }

    std::unordered_map<std::string, T> &globalScope() { return *scopes_.front(); }

    const std::unordered_map<std::string, T> &globalScope() const { return *scopes_.front(); }
};
