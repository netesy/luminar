#include "function.hh"
#include "builtin_function.hh"
#include "variable.hh"
#include <algorithm> // For std::count_if

// ParameterInfo constructor
ParameterInfo::ParameterInfo(const std::string &n, TypePtr t, bool opt, ValuePtr def)
    : name(n), type(t), isOptional(opt), defaultValue(def) {}

// ParameterStackFrame constructor
ParameterStackFrame::ParameterStackFrame(const std::string &name)
    : functionName(name) {}

// FunctionInfo implementations
void FunctionInfo::initVariableMap() {
    for (const auto& param : parameters) {
        variableMap[param.name] = true;
    }

    if (!functionBody.empty()) {
        for (const auto& instr : functionBody) {
            if (instr.opcode == Opcode::STORE_VARIABLE) {
                // This logic is currently flawed and will be addressed separately.
                // For now, it is commented out to allow compilation.
            }
        }
    }
}

FunctionInfo::FunctionInfo()
    : name(""), parameters(), returnType(nullptr), startPC(-1), endPC(-1),
      isBuiltin(false), nativeImpl(nullptr), requiredParamCount(0), functionBody() {}

FunctionInfo::FunctionInfo(const std::string &n,
                             const std::vector<ParameterInfo> &params,
                             TypePtr ret,
                             int32_t start,
                             int32_t end,
                             size_t reqCount,
                             std::vector<Instruction> body)
    : name(n), parameters(params), returnType(ret), startPC(start), endPC(end),
      isBuiltin(false), nativeImpl(nullptr), requiredParamCount(reqCount),
      functionBody(std::move(body)) {
    initVariableMap();
}

FunctionInfo::FunctionInfo(const std::string& n,
            const std::vector<ParameterInfo>& params,
            TypePtr ret,
            std::function<ValuePtr(const std::vector<ValuePtr>&)> impl,
            size_t reqCount)
    : name(n), parameters(params), returnType(ret), startPC(-1), endPC(-1),
      isBuiltin(true), nativeImpl(impl), requiredParamCount(reqCount) {
    initVariableMap();
}

FunctionInfo::FunctionInfo(const FunctionInfo &other)
    : name(other.name), parameters(other.parameters), returnType(other.returnType),
      startPC(other.startPC), endPC(other.endPC.load()), isBuiltin(other.isBuiltin),
      nativeImpl(other.nativeImpl), requiredParamCount(other.requiredParamCount),
      functionBody(other.functionBody), variableMap(other.variableMap) {}

FunctionInfo &FunctionInfo::operator=(const FunctionInfo &other) {
    if (this != &other) {
        name = other.name;
        parameters = other.parameters;
        returnType = other.returnType;
        startPC = other.startPC;
        endPC.store(other.endPC.load());
        isBuiltin = other.isBuiltin;
        nativeImpl = other.nativeImpl;
        requiredParamCount = other.requiredParamCount;
        functionBody = other.functionBody;
        variableMap = other.variableMap;
    }
    return *this;
}

// Functions constructor
Functions::Functions(std::shared_ptr<TypeSystem> typeSystem)
    : typeSystem_(typeSystem), scopeManager_(), currentScopeId_(0),
      variable(typeSystem) {
    BuiltinFunctions<Functions>::registerWith(*this, typeSystem_);
}

void Functions::addFunction(const std::string &name,
                             const std::vector<ParameterInfo> &params,
                             TypePtr returnType,
                             int32_t startPC,
                             int32_t endPC,const std::vector<Instruction>& functionBody)
{
    size_t requiredCount = std::count_if(params.begin(),
                                         params.end(),
                                         [](const ParameterInfo &param) {
                                             return !param.isOptional;
                                         });

    validateParameters(params, name);

    FunctionInfo info(name, params, returnType, startPC, endPC, requiredCount, functionBody);

    if (scopeManager_.exists(name)) {
        throw std::runtime_error("Function already defined: " + name);
    }

    scopeManager_.add(name, info);
}

bool Functions::isParameter(const std::string& funcName, const std::string& varName) const {
    auto funcInfo = getFunction(funcName);
    if (!funcInfo) return false;

    auto it = funcInfo->variableMap.find(varName);
    return it != funcInfo->variableMap.end() && it->second;
}

void Functions::addBuiltinFunction(const std::string &name,
                        const std::vector<ParameterInfo> &params,
                        TypePtr returnType,
                        std::function<ValuePtr(const std::vector<ValuePtr> &)> implementation)
{
    size_t requiredCount = std::count_if(params.begin(),
                                         params.end(),
                                         [](const ParameterInfo &param) {
                                             return !param.isOptional;
                                         });

    validateParameters(params, name);

    FunctionInfo info(name, params, returnType, implementation, requiredCount);

    if (scopeManager_.exists(name)) {
        throw std::runtime_error("Function already defined: " + name);
    }

    scopeManager_.add(name, info);
}

std::optional<FunctionInfo> Functions::getFunction(const std::string &name) const
{
    const auto *funcInfo = scopeManager_.get(name);
    if (!funcInfo) {
        return std::nullopt;
    }
    return std::optional<FunctionInfo>(*funcInfo);
}

std::optional<FunctionInfo> Functions::getFunctionFromScope(
    const std::string &name, ScopeManager<FunctionInfo>::ScopeId scopeId) const
{
    const auto *funcInfo = scopeManager_.getFromScope(scopeId, name);
    if (!funcInfo) {
        return std::nullopt;
    }
    return std::optional<FunctionInfo>(*funcInfo);
}

void Functions::updateFunctionEndPC(const std::string &name, int32_t endPC)
{
    const auto *funcInfo = scopeManager_.get(name);
    if (!funcInfo) {
        throw std::runtime_error("Function not found: " + name);
    }
    const_cast<FunctionInfo *>(funcInfo)->endPC.store(endPC);
}

ScopeManager<FunctionInfo>::ScopeId Functions::enterScope()
{
    currentScopeId_ = scopeManager_.enterScope();
    return currentScopeId_;
}

void Functions::enterExistingScope(ScopeManager<FunctionInfo>::ScopeId scopeId)
{
    scopeManager_.enterExistingScope(scopeId);
    currentScopeId_ = scopeId;
}

void Functions::exitScope()
{
    scopeManager_.exitScope();
    currentScopeId_ = scopeManager_.getCurrentScopeId();
}

ScopeManager<FunctionInfo>::ScopeId Functions::getCurrentScopeId() const { return currentScopeId_; }

bool Functions::isInCurrentScope(const std::string &name) const
{
    return scopeManager_.existsInCurrentScope(name);
}

bool Functions::hasFunction(const std::string &name) const { return scopeManager_.exists(name); }

void Functions::validateFunctionCall(const std::string &name, const std::vector<ValuePtr> &arguments) const
{
    auto funcInfo = scopeManager_.get(name);
    if (!funcInfo) {
        throw std::runtime_error("Function not found: " + name);
    }

    if (arguments.size() < funcInfo->requiredParamCount
        || arguments.size() > funcInfo->parameters.size()) {
        throw std::runtime_error("Incorrect number of arguments for function: " + name);
    }

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (!typeSystem_->isCompatible(arguments[i]->type, funcInfo->parameters[i].type)) {
            throw std::runtime_error("Argument type mismatch for parameter '"
                                     + funcInfo->parameters[i].name + "' in function '" + name
                                     + "'");
        }
    }
}

ValuePtr Functions::executeBuiltin(const std::string &name, const std::vector<ValuePtr> &providedArgs) const
{
    auto funcInfo = scopeManager_.get(name);
    if (!funcInfo || !funcInfo->isBuiltin) {
        throw std::runtime_error("Builtin function not found or is not a builtin: " + name);
    }
    return funcInfo->nativeImpl(providedArgs);
}

void Functions::pushParameterFrame(const std::string &functionName, const std::vector<ValuePtr> &args)
{
    if (scopeManager_.getCurrentScopeDepth() < 0) {
        throw std::runtime_error("Scope manager not properly initialized");
    }

    if (scopeManager_.exists(functionName)) {
        auto funcInfo = scopeManager_.get(functionName);
        if (!funcInfo) {
            throw std::runtime_error("Cannot push parameters for undefined function: "
                                     + functionName);
        }

        auto frame = std::make_shared<ParameterStackFrame>(functionName);

        for (size_t i = 0; i < args.size(); ++i) {
            frame->parameters[funcInfo->parameters[i].name] = args[i];
        }

        for (size_t i = args.size(); i < funcInfo->parameters.size(); ++i) {
            const auto &param = funcInfo->parameters[i];
            if (param.isOptional) {
                frame->parameters[param.name] = param.defaultValue;
            }
        }

        parameterStack_.push(frame);
    }
}

void Functions::popParameterFrame()
{
    if (parameterStack_.empty()) {
        throw std::runtime_error("Cannot pop parameter frame: stack is empty");
    }
    parameterStack_.pop();
}

ValuePtr Functions::getParameter(const std::string &paramName) const
{
    if (parameterStack_.empty()) {
        throw std::runtime_error("No parameter found for active function call: "+ paramName);
    }

    const auto &currentFrame = parameterStack_.top();
    auto it = currentFrame->parameters.find(paramName);
    if (it == currentFrame->parameters.end()) {
        throw std::runtime_error("Parameter not found: " + paramName + " in function "
                                 + currentFrame->functionName);
    }
    return it->second;
}

std::unordered_map<std::string, ValuePtr> Functions::getCurrentParameters() const
{
    if (parameterStack_.empty()) {
        throw std::runtime_error("No parameters found for active function call");
    }
    return parameterStack_.top()->parameters;
}

std::string Functions::getCurrentFunctionName() const
{
    if (parameterStack_.empty()) {
        throw std::runtime_error("No active function call");
    }
    return parameterStack_.top()->functionName;
}

bool Functions::hasParameter(const std::string &paramName) const
{
    if (parameterStack_.empty()) {
        return false;
    }
    return parameterStack_.top()->parameters.count(paramName) > 0;
}

size_t Functions::getParameterStackDepth() const { return parameterStack_.size(); }

std::vector<ValuePtr> Functions::prepareArguments(const std::string &name,
                                       const std::vector<ValuePtr> &providedArgs) const
{
    auto funcInfo = scopeManager_.get(name);
    if (!funcInfo) {
        throw std::runtime_error("Function not found: " + name);
    }

    validateFunctionCall(name, providedArgs);

    std::vector<ValuePtr> finalArgs;
    finalArgs.reserve(funcInfo->parameters.size());

    for (size_t i = 0; i < providedArgs.size(); i++) {
        finalArgs.push_back(providedArgs[i]);
    }

    for (size_t i = providedArgs.size(); i < funcInfo->parameters.size(); i++) {
        const auto &param = funcInfo->parameters[i];
        if (!param.isOptional) {
            throw std::runtime_error("Internal error: required parameter after optional ones");
        }
        finalArgs.push_back(param.defaultValue);
    }

    return finalArgs;
}

std::optional<std::vector<Instruction>> Functions::getFunctionBody(const std::string& name) const {
    const auto* funcInfo = scopeManager_.get(name);
    if (!funcInfo || funcInfo->isBuiltin) {
        return std::nullopt;
    }

    return funcInfo->functionBody;
}

void Functions::validateParameters(const std::vector<ParameterInfo>& params, const std::string& functionName) {
    bool foundOptional = false;
    for (const auto& param : params) {
        if (foundOptional && !param.isOptional) {
            throw std::runtime_error("Required parameters must come before optional parameters in function: " + functionName);
        }
        if (param.isOptional) {
            foundOptional = true;
            if (param.defaultValue && !typeSystem_->checkType(param.defaultValue, param.type)) {
                throw std::runtime_error("Default value type mismatch for parameter '" + param.name + "' in function '" + functionName + "'");
            }
        }
    }
}

