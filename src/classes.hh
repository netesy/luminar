#pragma once
#include "function.hh"
#include "value.hh"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>

// Decorator support
struct MethodDecorator {
    std::function<FunctionInfo(FunctionInfo)> transform;
    std::string name;
};

// Metaclass support
struct MetaclassInfo {
    std::string name;
    std::vector<std::string> bases;
    std::unordered_map<std::string, ValuePtr> classAttributes;
    std::function<ValuePtr(const std::string&, const std::vector<TypePtr>&)> classCreator;
    std::vector<MethodDecorator> methodDecorators;
};

class ClassManager {
private:
    std::shared_ptr<Functions> functions_;
    std::shared_ptr<TypeSystem> typeSystem_;
    std::unordered_map<std::string, std::vector<FunctionInfo>> classMethods_;
    std::unordered_map<std::string, MetaclassInfo> metaclasses_;
    std::unordered_map<std::string, std::vector<std::string>> inheritance_;
    std::unordered_map<std::string, std::unordered_set<std::string>> interfaces_;
    std::unordered_map<std::string, std::vector<std::string>> mixins_;

public:
    ClassManager(std::shared_ptr<Functions> functions, std::shared_ptr<TypeSystem> typeSystem)
        : functions_(functions), typeSystem_(typeSystem) {}

    void registerMetaclass(const std::string& name, const MetaclassInfo& meta) {
        metaclasses_[name] = meta;
    }

    void registerInterface(const std::string& name, const std::vector<FunctionInfo>& methods) {
        if (interfaces_.find(name) != interfaces_.end()) {
            throw std::runtime_error("Interface already registered: " + name);
        }
        registerClass(name, methods);
        interfaces_[name] = std::unordered_set<std::string>();
    }

    void registerMixin(const std::string& name, 
                      const std::vector<FunctionInfo>& methods,
                      const std::vector<std::string>& requiredMethods) {
        if (mixins_.find(name) != mixins_.end()) {
            throw std::runtime_error("Mixin already registered: " + name);
        }
        registerClass(name, methods);
        mixins_[name] = requiredMethods;
    }

    void registerClass(const std::string& className, 
                      const std::vector<FunctionInfo>& methods,
                      const std::string& metaclassName = "type",
                      const std::vector<std::string>& bases = {},
                      const std::vector<std::string>& implements = {},
                      const std::vector<std::string>& with = {}) {
if (classMethods_.find(className) != classMethods_.end()) {
        throw std::runtime_error("Class already registered: " + className);
    }

    // Store base classes
    inheritance_[className] = bases;

    // Verify and store interfaces
    for (const auto& interface : implements) {
        if (!isInterface(interface)) {
            throw std::runtime_error("Unknown interface: " + interface);
        }
        // Verify all interface methods are implemented
        for (const auto& method : classMethods_[interface]) {
            bool found = false;
            for (const auto& classMethod : methods) {
                if (method.name == classMethod.name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Missing interface method: " + method.name);
            }
        }
        interfaces_[interface].insert(className);
    }

    // Apply mixins
    auto finalMethods = methods;
    for (const auto& mixin : with) {
        if (!isMixin(mixin)) {
            throw std::runtime_error("Unknown mixin: " + mixin);
        }
        // Verify required methods
        for (const auto& required : mixins_[mixin]) {
            bool found = false;
            for (const auto& method : finalMethods) {
                if (required == method.name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Missing required method for mixin: " + required);
            }
        }
        // Add mixin methods
        const auto& mixinMethods = classMethods_[mixin];
        finalMethods.insert(finalMethods.end(), mixinMethods.begin(), mixinMethods.end());
    }

    // Apply metaclass if specified
    if (metaclasses_.find(metaclassName) != metaclasses_.end()) {
        const auto& meta = metaclasses_[metaclassName];
        // Apply method decorators
        for (auto& method : finalMethods) {
            for (const auto& decorator : meta.methodDecorators) {
                method = decorator.transform(method);
            }
        }
    }

    // Store final methods
    classMethods_[className] = finalMethods;

    // Register class type
    auto classType = std::make_shared<Type>(TypeTag::UserDefined);
    auto& userType = classType->extra.emplace<UserDefinedType>();
    userType.name = className;

    // Register methods in function manager
    for (const auto& method : finalMethods) {
        if (method.isBuiltin) {
            functions_->addBuiltinFunction(
                className + "::" + method.name,
                method.parameters,
                method.returnType,
                method.nativeImpl
            );
        } else {
            functions_->addFunction(
                className + "::" + method.name,
                method.parameters,
                method.returnType,
                method.startPC,
                method.endPC.load(),
                method.functionBody
            );
        }
    }
    }

    bool hasMethod(const std::string& className, const std::string& methodName) {
        // Check own methods
        auto it = classMethods_.find(className);
        if (it != classMethods_.end()) {
            return std::any_of(it->second.begin(), it->second.end(),
                [&methodName](const FunctionInfo& method) {
                    return method.name == methodName;
                });
        }
        // Check base classes
        auto baseIt = inheritance_.find(className);
        if (baseIt != inheritance_.end()) {
            for (const auto& base : baseIt->second) {
                if (hasMethod(base, methodName)) return true;
            }
        }
        return false;
    }

    FunctionInfo getMethod(const std::string& className, const std::string& methodName) {
    // Check in current class
    auto classIt = classMethods_.find(className);
    if (classIt != classMethods_.end()) {
        for (const auto& method : classIt->second) {
            if (method.name == methodName) {
                return method;
            }
        }
    }

    // Check base classes (depth-first)
    auto baseIt = inheritance_.find(className);
    if (baseIt != inheritance_.end()) {
        for (const auto& base : baseIt->second) {
            try {
                return getMethod(base, methodName);
            } catch (const std::runtime_error&) {
                continue;
            }
        }
    }

    // Check mixins
    auto mixinIt = mixins_.find(className);
    if (mixinIt != mixins_.end()) {
        for (const auto& mixin : mixinIt->second) {
            try {
                return getMethod(mixin, methodName);
            } catch (const std::runtime_error&) {
                continue;
            }
        }
    }

    // Check interfaces
    for (const auto& [interface, implementers] : interfaces_) {
        if (implementers.find(className) != implementers.end()) {
            try {
                return getMethod(interface, methodName);
            } catch (const std::runtime_error&) {
                continue;
            }
        }
    }

    throw std::runtime_error("Method not found: " + methodName + " in class " + className);
    }

    bool isInterface(const std::string& name) const {
        return interfaces_.find(name) != interfaces_.end();
    }

    bool isMixin(const std::string& name) const {
        return mixins_.find(name) != mixins_.end();
    }

    std::vector<std::string> getBaseClasses(const std::string& className) const {
        auto it = inheritance_.find(className);
        return it != inheritance_.end() ? it->second : std::vector<std::string>();
    }
};
