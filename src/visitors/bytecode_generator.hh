// src/visitors/bytecode_generator.hh
#pragma once

#include "../ast.hh"
#include "../opcodes.hh"
#include "../value.hh"
#include "../instructions.hh"
#include "../function.hh"
#include "../variable.hh"
#include "../types.hh"

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <stack>
#include <algorithm>


// Forward declarations
struct Upvalue {
    uint8_t index;
    bool isLocal;

    Upvalue(uint8_t idx = 0, bool local = false) : index(idx), isLocal(local) {}
};

class BytecodeGenerator : public ASTVisitor {
public:
    BytecodeGenerator(TypeSystem& typeSystem)
        : typeSystem_(std::make_shared<TypeSystem>(typeSystem)),
        functions_(std::make_shared<Functions>(typeSystem_)),
        variables_(std::make_shared<Variables>(typeSystem_)) {
        // Initialize with global scope
        scopes.push(Scope{false, variables_});
    }

    const std::vector<Instruction>& getBytecode() const {
        return bytecode;
    }

    /**
     * @brief Emits a new instruction with the given opcode, line number, and value
     * @param opcode The opcode of the instruction
     * @param lineNumber The source line number for debugging
     * @param value The value to be associated with the instruction
     * @return The created instruction
     */
    Instruction emit(Opcode opcode, uint32_t lineNumber, ValuePtr value = nullptr) {
        Instruction instruction(opcode, lineNumber, value);
        bytecode.push_back(instruction);
        return instruction;
    }

    // Overload for direct Value creation
    Instruction emit(Opcode opcode, uint32_t lineNumber, Value&& value) {
        ValuePtr valuePtr = std::make_shared<Value>(std::move(value));
        return emit(opcode, lineNumber, valuePtr);
    }

    // Inherit all visit methods from ASTVisitor
    using ASTVisitor::visit;

private:
    struct Scope {
        std::unordered_map<std::string, VariableInfo> variables;
        bool isFunction = false;
        int nextVarIndex = 0;
        std::shared_ptr<Variables> variablesRef;

        explicit Scope(bool isFunc = false, std::shared_ptr<Variables> vars = nullptr)
            : isFunction(isFunc), variablesRef(vars) {}

        void addVariable(const std::string& name, TypePtr type = nullptr, bool isInitialized = false) {
            if (!variablesRef) {
                throw std::runtime_error("Variables reference not set in Scope");
            }

            ValuePtr initialValue = nullptr;
            if (isInitialized) {
                initialValue = std::make_shared<Value>(type);
            }
            int32_t memoryLocation = variablesRef->addVariable(name, type, false, initialValue);

            VariableInfo info{
                memoryLocation,
                true,
                initialValue,
                type
            };

            variables.emplace(name, info);
        }

        std::optional<VariableInfo> findVariable(const std::string& name) const {
            auto it = variables.find(name);
            if (it != variables.end()) {
                return it->second;
            }
            return std::nullopt;
        }
    };

    struct LoopContext {
        size_t startLabel;
        size_t breakLabel;
    };

    // Core state
    std::shared_ptr<TypeSystem> typeSystem_;
    std::shared_ptr<Functions> functions_;
    std::shared_ptr<Variables> variables_;
    std::vector<Instruction> bytecode;
    std::vector<Upvalue> m_upvalues;
    std::vector<ValuePtr> constants;
    std::vector<std::string> stringPool;
    std::stack<Scope> scopes;
    std::stack<LoopContext> loopStack;
    std::vector<std::vector<size_t>> breakTargets;
    std::vector<std::vector<size_t>> continueTargets;
    std::unordered_map<std::string, std::vector<size_t>> pendingJumps;
    std::unordered_map<std::string, size_t> labelPositions;
    int scopeDepth = 0;
    bool isLastStatementInBlock = false;

public:
    // Scope management
    void enterScope(bool isFunction = false) {
        Scope newScope(isFunction, variables_);
        if (!scopes.empty() && !isFunction) {
            newScope.nextVarIndex = scopes.top().nextVarIndex;
        }
        scopes.push(std::move(newScope));

        if (isFunction) {
            scopeDepth++;
        }
    }

    void leaveScope(bool isFunction = false) {
        if (scopes.empty()) return;

        if (isFunction && scopeDepth > 0) {
            scopeDepth--;
        }
        scopes.pop();
    }

    Scope& currentScope() {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }
        return scopes.top();
    }

    const Scope& currentScope() const {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }
        return scopes.top();
    }

    // Variable management
    std::optional<VariableInfo> findVariable(const std::string& name) {
        if (scopes.empty()) {
            return std::nullopt;
        }

        // Check current scope
        if (auto var = scopes.top().findVariable(name)) {
            return var;
        }

        // Check variables table
        if (variables_ && variables_->hasVariable(name)) {
            return VariableInfo{
                variables_->getVariableMemoryLocation(name),
                true,
                variables_->getVariableValue(name),
                variables_->getVariableType(name)
            };
        }

        return std::nullopt;
    }

    void addToCurrentScope(const std::string& name, bool isInitialized = false) {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }
        scopes.top().addVariable(name, nullptr, isInitialized);
    }

    // Constant management
    size_t addConstant(const Value& val) {
        // First check if we already have this constant
        for (size_t i = 0; i < constants.size(); ++i) {
            if (constants[i] && constants[i]->toString() == val.toString()) {
                return i;
            }
        }

        try {
            constants.emplace_back(std::make_shared<Value>(val));
            return constants.size() - 1;
        } catch (const std::exception& e) {
            std::cerr << "Error creating constant value: " << e.what() << std::endl;
            throw;
        }
    }

    size_t addString(const std::string& str) {
        auto it = std::find(stringPool.begin(), stringPool.end(), str);
        if (it != stringPool.end()) {
            return std::distance(stringPool.begin(), it);
        }
        stringPool.push_back(str);
        return stringPool.size() - 1;
    }

    // Visitor implementations
    void visit(NumberNode& node) override {
        size_t constIndex = addConstant(node.value);
        emit(Opcode::LOAD_CONST, node.location.line, constants[constIndex]);
    }

    void visit(StringLiteralNode& node) override {
        size_t constIndex = addConstant(node.value);
        emit(Opcode::LOAD_CONST, node.location.line, constants[constIndex]);
    }

    void visit(BooleanNode& node) override {
        size_t constIndex = addConstant(node.value);
        emit(Opcode::LOAD_CONST, node.location.line, constants[constIndex]);
    }

    void visit(NilNode& node) override {
        emit(Opcode::LOAD_VALUE, node.location.line);
    }

    void visit(VariableNode& node) override {
        const std::string& name = node.getName();
        auto varInfo = findVariable(name);
        if (!varInfo) {
            throw std::runtime_error("Undefined variable: " + name);
        }

        auto type = std::make_shared<Type>(node.type.tag);
        auto value = std::make_shared<Value>(type, static_cast<int64_t>(varInfo->memoryLocation));
        emit(Opcode::LOAD_VARIABLE, node.location.line, value);
    }

    void visit(BinaryNode& node) override {
        // Visit operands
        node.left->accept(*this);
        node.right->accept(*this);

        // Emit operation
        if (node.operatorType == "+") {
            emit(Opcode::ADD, node.location.line);
        } else if (node.operatorType == "-") {
            emit(Opcode::SUBTRACT, node.location.line);
        } else if (node.operatorType == "*") {
            emit(Opcode::MULTIPLY, node.location.line);
        } else if (node.operatorType == "/") {
            emit(Opcode::DIVIDE, node.location.line);
        } else if (node.operatorType == "==") {
            emit(Opcode::EQUAL, node.location.line);
        } else if (node.operatorType == "!=") {
            emit(Opcode::NOT_EQUAL, node.location.line);
        } else if (node.operatorType == "<") {
            emit(Opcode::LESS_THAN, node.location.line);
        } else if (node.operatorType == "<=") {
            emit(Opcode::LESS_THAN_OR_EQUAL, node.location.line);
        } else if (node.operatorType == ">") {
            emit(Opcode::GREATER_THAN, node.location.line);
        } else if (node.operatorType == ">=") {
            emit(Opcode::GREATER_THAN_OR_EQUAL, node.location.line);
        } else if (node.operatorType == "&&") {
            emit(Opcode::AND, node.location.line);
        } else if (node.operatorType == "||") {
            emit(Opcode::OR, node.location.line);
        } else {
            throw std::runtime_error("Unsupported binary operator: " + node.operatorType);
        }
    }

    void visit(UnaryNode& node) override {
        // Visit operand first
        node.operand->accept(*this);

        // Emit operation
        if (node.operatorType == "-") {
            emit(Opcode::NEGATE, node.location.line);
        } else if (node.operatorType == "!") {
            emit(Opcode::NOT, node.location.line);
        } else {
            throw std::runtime_error("Unsupported unary operator: " + node.operatorType);
        }
    }

    void visit(AssignmentNode& node) override {
        // Generate code for right-hand side
        node.right->accept(*this);

        // Handle assignment target
        if (auto varNode = dynamic_cast<VariableNode*>(node.left.get())) {
            auto varInfo = findVariable(varNode->getName());
            if (!varInfo) {
                // Add new variable to current scope
                addToCurrentScope(varNode->getName(), true);
                varInfo = findVariable(varNode->getName());
            } else if (!varInfo->isMutable) {
                throw std::runtime_error("Cannot assign to immutable variable: " + varNode->getName());
            }

            auto type = std::make_shared<Type>(TypeTag::Int);
            auto value = std::make_shared<Value>(type, static_cast<int64_t>(varInfo->memoryLocation));
            emit(Opcode::STORE_VARIABLE, node.location.line, value);
        } else {
            throw std::runtime_error("Invalid left-hand side of assignment");
        }
    }

    void visit(CallNode& node) override {
        throw std::runtime_error("CallNode visit not implemented");
    }

    void visit(GroupingNode& node) override {
        node.expression->accept(*this);
    }

    void visit(ConditionalNode& node) override {
        // Generate condition
        node.condition->accept(*this);

        // Jump if false - placeholder value
        size_t jumpIfFalsePos = bytecode.size();
        emit(Opcode::JUMP_IF_FALSE, node.location.line);

        // Then branch
        node.thenBranch->accept(*this);

        // Jump over else - placeholder value
        size_t jumpPos = bytecode.size();
        emit(Opcode::JUMP, node.location.line);

        // Update first jump target
        auto type = std::make_shared<Type>(TypeTag::Int);
        auto jumpTarget = std::make_shared<Value>(type, static_cast<int64_t>(bytecode.size()));
        bytecode[jumpIfFalsePos].value = jumpTarget;

        // Else branch
        if (node.elseBranch) {
            (*node.elseBranch)->accept(*this);
        }

        // Update second jump target
        auto jumpTarget2 = std::make_shared<Value>(type, static_cast<int64_t>(bytecode.size()));
        bytecode[jumpPos].value = jumpTarget2;
    }

    void visit(WhileNode& node) override {
        breakTargets.emplace_back();
        continueTargets.emplace_back();

        size_t loopStart = bytecode.size();

        // Condition
        node.condition->accept(*this);

        // Exit jump
        size_t exitJump = bytecode.size();
        emit(Opcode::JUMP_IF_FALSE, node.location.line);

        // Body
        node.body->accept(*this);

        // Jump back
        auto type = std::make_shared<Type>(TypeTag::Int);
        auto jumpBack = std::make_shared<Value>(type, static_cast<int64_t>(loopStart));
        emit(Opcode::JUMP, node.location.line, jumpBack);

        // Patch jumps
        size_t afterLoop = bytecode.size();
        auto jumpTarget = std::make_shared<Value>(type, static_cast<int64_t>(afterLoop));
        bytecode[exitJump].value = jumpTarget;

        // Patch continue jumps
        for (size_t continuePos : continueTargets.back()) {
            auto continueTarget = std::make_shared<Value>(type, static_cast<int64_t>(loopStart));
            bytecode[continuePos].value = continueTarget;
        }

        // Patch break jumps
        for (size_t breakPos : breakTargets.back()) {
            auto breakTarget = std::make_shared<Value>(type, static_cast<int64_t>(afterLoop));
            bytecode[breakPos].value = breakTarget;
        }

        breakTargets.pop_back();
        continueTargets.pop_back();
    }

    void visit(ForNode& node) override {
        breakTargets.emplace_back();
        continueTargets.emplace_back();
        enterScope();

        try {
            // Initializer
            if (node.initializer) {
                node.initializer->accept(*this);
            }

            size_t conditionStart = bytecode.size();
            size_t jumpToEnd = 0;

            // Condition
            if (node.condition) {
                node.condition->accept(*this);
                jumpToEnd = bytecode.size();
                emit(Opcode::JUMP_IF_FALSE, node.location.line);
            }

            // Body
            node.body->accept(*this);

            size_t incrementStart = bytecode.size();

            // Increment
            if (node.increment) {
                node.increment->accept(*this);
            }

            // Jump back
            auto type = std::make_shared<Type>(TypeTag::Int);
            auto jumpBack = std::make_shared<Value>(type, static_cast<int64_t>(conditionStart));
            emit(Opcode::JUMP, node.location.line, jumpBack);

            size_t afterLoop = bytecode.size();

            // Patch condition jump
            if (node.condition) {
                auto jumpTarget = std::make_shared<Value>(type, static_cast<int64_t>(afterLoop));
                bytecode[jumpToEnd].value = jumpTarget;
            }

            // Patch continue/break jumps
            size_t continueTarget = node.increment ? incrementStart : conditionStart;
            for (size_t continuePos : continueTargets.back()) {
                auto target = std::make_shared<Value>(type, static_cast<int64_t>(continueTarget));
                bytecode[continuePos].value = target;
            }
            for (size_t breakPos : breakTargets.back()) {
                auto target = std::make_shared<Value>(type, static_cast<int64_t>(afterLoop));
                bytecode[breakPos].value = target;
            }

            breakTargets.pop_back();
            continueTargets.pop_back();
            leaveScope();
        } catch (...) {
            breakTargets.pop_back();
            continueTargets.pop_back();
            leaveScope();
            throw;
        }
    }

    void visit(BlockNode& node) override {
        enterScope();
        for (const auto& stmt : node.getStatements()) {
            stmt->accept(*this);
        }
        leaveScope();
    }

    void visit(ReturnNode& node) override {
        if (node.value) {
            node.value->accept(*this);
        } else {
            emit(Opcode::LOAD_STR, node.location.line);
        }
        emit(Opcode::RETURN, node.location.line);
    }

    void visit(FunctionNode& node) override {
        // Save current bytecode
        std::vector<Instruction> originalBytecode = std::move(bytecode);
        bytecode.clear();

        // Convert parameters
        std::vector<ParameterInfo> parameters;
        for (const auto& param : node.parameters) {
            parameters.emplace_back(
                param.name,
                std::make_shared<Type>(param.type),
                false
                );
        }

        TypePtr returnType = std::make_shared<Type>(node.returnType.tag);

        // Enter function scope
        enterScope(true);

        // Add parameters to scope
        for (const auto& param : node.parameters) {
            addToCurrentScope(param.name, true);
        }

        // Generate body
        for (auto& stmt : node.body) {
            stmt->accept(*this);
        }

        // Add implicit return
        if (bytecode.empty() || bytecode.back().opcode != Opcode::RETURN) {
            emit(Opcode::RETURN, node.location.line);
        }

        // Store function
        functions_->addFunction(node.name, parameters, returnType, 0, -1, bytecode);

        leaveScope(true);

        // Restore bytecode and emit function definition
        bytecode = std::move(originalBytecode);
        auto type = std::make_shared<Type>(TypeTag::String);
        auto funcNameValue = std::make_shared<Value>(type, node.name);
        emit(Opcode::DEFINE_FUNCTION, node.location.line, funcNameValue);
    }

    void visit(LambdaNode& node) override {
        throw std::runtime_error("LambdaNode visit not implemented");
    }

    void visit(MatchNode& node) override {
        throw std::runtime_error("MatchNode visit not implemented");
    }

    void visit(FieldNode& node) override {
        throw std::runtime_error("FieldNode visit not implemented");
    }

    void visit(ModuleNode& node) override {
        if (node.body) {
            node.body->accept(*this);
        }
    }

    void visit(ErrorHandlingNode& node) override {
        node.expression->accept(*this);
    }

    void visit(StreamProcessingNode& node) override {
        throw std::runtime_error("StreamProcessingNode visit not implemented");
    }

    void visit(AtomicNode& node) override {
        throw std::runtime_error("AtomicNode visit not implemented");
    }

    void visit(ChannelNode& node) override {
        throw std::runtime_error("ChannelNode visit not implemented");
    }

    void visit(ImportNode& node) override {
        throw std::runtime_error("ImportNode visit not implemented");
    }

    void visit(InterfaceNode& node) override {
        throw std::runtime_error("InterfaceNode visit not implemented");
    }

    void visit(MixinNode& node) override {
        throw std::runtime_error("MixinNode visit not implemented");
    }

    void visit(UnsafeNode& node) override {
        throw std::runtime_error("UnsafeNode visit not implemented");
    }

    void visit(InterpolatedStringNode& node) override {
        auto type = std::make_shared<Type>(TypeTag::String);
        auto emptyString = std::make_shared<Value>(type, std::string(""));
        emit(Opcode::LOAD_CONST, node.location.line, emptyString);

        for (const auto& part : node.parts) {
            part->accept(*this);
            emit(Opcode::ADD, node.location.line);
        }
    }

    void visit(RangeNode& node) override {
        node.start->accept(*this);
        node.end->accept(*this);

        if (node.step) {
            node.step->accept(*this);
        } else {
            auto type = std::make_shared<Type>(TypeTag::Int);
            auto oneValue = std::make_shared<Value>(type, static_cast<int64_t>(1));
            emit(Opcode::LOAD_CONST, node.location.line, oneValue);
        }

        emit(Opcode::MAKE_RANGE, node.location.line);
    }

    void visit(ListNode& node) override {
        throw std::runtime_error("ListNode visit not implemented");
    }

    void visit(DictNode& node) override {
        throw std::runtime_error("DictNode visit not implemented");
    }

    void visit(ConcurrentNode& node) override {}
    void visit(ParallelNode& node) override {}
    void visit(ClassNode& node) override {}

    std::vector<Value> getConstants() const {
        std::vector<Value> result;
        result.reserve(constants.size());
        for (const auto& ptr : constants) {
            if (ptr) {
                result.push_back(*ptr);
            } else {
                result.emplace_back();
            }
        }
        return result;
    }

    const std::vector<std::string>& getStringPool() const { return stringPool; }

private:
    void updateLabelJumps(const std::string& label, size_t targetPos) {
        auto it = pendingJumps.find(label);
        if (it != pendingJumps.end()) {
            for (size_t jumpPos : it->second) {
                int offset = static_cast<int>(targetPos) - static_cast<int>(jumpPos) - 1;
                auto type = std::make_shared<Type>(TypeTag::Int);
                auto offsetValue = std::make_shared<Value>(type, static_cast<int64_t>(offset));
                bytecode[jumpPos].value = offsetValue;
            }
            pendingJumps.erase(it);
        }
    }
};
