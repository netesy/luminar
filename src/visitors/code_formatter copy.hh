// src/visitors/code_formatter.hh
#pragma once

#include "../ast.hh"
#include <string>
#include <vector>
#include <sstream>

class CodeFormatter : public ASTVisitor {
public:
    virtual ~CodeFormatter() = default;
    virtual std::string format(ASTNode& node) = 0;

protected:
    std::ostringstream output;
    std::shared_ptr<TypeSystem> typeSystem;
    int indentLevel = 0;
    bool atLineStart = true;
    bool needsSemicolon = true;

    void addIndent() {
        if (atLineStart) {
            output << std::string(indentLevel * 4, ' ');
            atLineStart = false;
        }
    }

    void newLine() {
        output << '\n';
        atLineStart = true;
        needsSemicolon = true;
    }

    void write(const std::string& str) {
        addIndent();
        output << str;
    }

public:
    explicit CodeFormatter(std::shared_ptr<TypeSystem> ts) : typeSystem(std::move(ts)) {}

    std::string format(ASTNode& node) {
        output.str("");  // Clear any existing content
        output.clear();   // Clear any error flags
        node.accept(*this);
        return output.str();
    }

    void visit(NumberNode& node) override {
        // Access the numeric value from the Value variant
        if (std::holds_alternative<int32_t>(node.value.data)) {
            output << std::get<int32_t>(node.value.data);
        } else if (std::holds_alternative<int64_t>(node.value.data)) {
            output << std::get<int64_t>(node.value.data);
        } else if (std::holds_alternative<double>(node.value.data)) {
            output << std::get<double>(node.value.data);
        } else if (std::holds_alternative<float>(node.value.data)) {
            output << std::get<float>(node.value.data);
        } else {
            output << "0"; // Default fallback
        }
    }

    void visit(StringLiteralNode& node) override {
        if (std::holds_alternative<std::string>(node.value.data)) {
            output << "\"" << std::get<std::string>(node.value.data) << "\"";
        } else {
            output << "\"\""; // Empty string as fallback
        }
    }

    void visit(BinaryNode& node) override {
        output << "(";
        node.left->accept(*this);
        output << " " << node.operatorType << " ";
        node.right->accept(*this);
        output << ")";
    }

    void visit(BlockNode& node) override {
        output << "{\n";
        indentLevel++;

        for (const auto& stmt : node.getStatements()) {
            (*stmt).accept(*this);
            if (needsSemicolon) {
                output << ";";
            }
            newLine();
        }

        indentLevel--;
        addIndent();
        output << "}";
        needsSemicolon = false;
    }

    void visit(VariableNode& node) override {
        write((node.isMutable() ? "var " : "const ") + node.getName());

        // Get the variable's type
        TypePtr varType = node.inferType(*typeSystem);

        // Always show type if it's explicitly specified or can't be inferred
        if (varType && varType->tag != TypeTag::Any) {
            output << ": " << varType->toString();
        }

        if (node.hasInitializer()) {
            output << " = ";
            node.getInitializer()->accept(*this);

            // Show inferred type if different from declared type
            if (Expression* initExpr = node.getInitializer()) {
                TypePtr initType = initExpr->inferType(*typeSystem);
                if (initType && initType->tag != TypeTag::Any &&
                    (!varType || initType->tag != varType->tag)) {
                    output << " /* : " << initType->toString() << " */";
                }
            }
        }
    }

    void visit(ConditionalNode& node) override {
        write("if (");
        node.condition->accept(*this);
        output << ") ";
        node.thenBranch->accept(*this);

        // Handle elif branches
        for (const auto& elif : node.elifBranches) {
            output << " elif (";
            elif.first->accept(*this);
            output << ") ";
            elif.second->accept(*this);
        }

        // Handle else branch
        if (node.elseBranch) {
            output << " else ";
            (*node.elseBranch)->accept(*this);
        }
        needsSemicolon = false;
    }

    void visit(WhileNode& node) override {
        write("while (");
        node.condition->accept(*this);
        output << ") ";
        node.body->accept(*this);
        needsSemicolon = false;
    }

    void visit(FunctionNode& node) override {
        write("fn " + node.name + "(");
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            if (i > 0) output << ", ";
            output << node.parameters[i].name;

            // Always show parameter type
            output << ": " << node.parameters[i].type.toString();

            // Add default value if present
            if (node.parameters[i].defaultValue) {
                output << " = ";
                output << node.parameters[i].defaultValue->toString();

                // Get the parameter type and actual default value type
                TypePtr paramType = std::make_shared<Type>(node.parameters[i].type);
                TypePtr defaultType = std::make_shared<Type>(node.parameters[i].defaultValue->type);

                // Show type information if types don't match
                if (paramType && defaultType && paramType->tag != defaultType->tag) {
                    output << " /* : " << defaultType->toString() << " */";
                }
            }
        }
        output << ")";

        // Only show return type if it's not nil
        if (node.returnType.tag != TypeTag::Nil) {
            output << " -> " << node.returnType.toString();
        }
        output << " ";

        // Handle function body as a block
        output << "{\n";
        indentLevel++;

        for (auto& stmt : node.body) {
            (*stmt).accept(*this);
            if (needsSemicolon) {
                output << ";";
            }
            newLine();
        }

        indentLevel--;
        write("}");
        needsSemicolon = false;
    }

    void visit(ReturnNode& node) override {
        write("return");
        if (node.value) {
            output << " ";
            node.value->accept(*this);

            // Show return type if it can be inferred
            if (Expression* returnExpr = dynamic_cast<Expression*>(node.value.get())) {
                TypePtr returnType = returnExpr->inferType(*typeSystem);
                if (returnType && returnType->tag != TypeTag::Any) {
                    output << " /* : " << returnType->toString() << " */";
                }
            }
        }
    }

    void visit(ForNode& node) override {
        write("for (") ;
        if (node.initializer) {
            node.initializer->accept(*this);
        }
        output << "; ";
        if (node.condition) {
            node.condition->accept(*this);
        }
        output << "; ";
        if (node.increment) {
            node.increment->accept(*this);
        }
        output << ") ";
        node.body->accept(*this);
        needsSemicolon = false;
    }

    void visit(RangeNode& node) override {
        output << "(";
        node.start->accept(*this);
        output << "..";
        node.end->accept(*this);
        if (node.hasStep) {
            output << "..";
            node.step->accept(*this);
        }
        output << ")";
    }

    void visit(ListNode& node) override {
        output << "[";
        for (size_t i = 0; i < node.elements.size(); ++i) {
            if (i > 0) output << ", ";
            node.elements[i]->accept(*this);
        }
        output << "]";
    }

    void visit(DictNode& node) override {
        output << "{";
        bool first = true;
        for (const auto& [key, value] : node.entries) {
            if (!first) output << ", ";
            output << "\"" << key << "\": ";
            value->accept(*this);
            first = false;
        }
        output << "}";
    }

    // Other required visitors with minimal implementations
    void visit(ConcurrentNode&) override { write("concurrent {}"); }
    void visit(ParallelNode&) override { write("parallel {}"); }

    void visit(ClassNode& node) override {
        write("class " + node.name);
        if (node.baseClass) {
            output << " : " << *node.baseClass;
        }
        output << " {\n";
        indentLevel++;
        for (auto& member : node.members) {
            member->accept(*this);
            output << "\n";
        }
        indentLevel--;
        write("}");
        needsSemicolon = false;
    }

    std::string getOutput() const {
        std::string result = output.str();
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        return result;
    }

    void clear() {
        output.str("");
        output.clear();
        indentLevel = 0;
        atLineStart = true;
        needsSemicolon = true;
    }

    // Missing method implementations
    void visit(ModuleNode& node) override {
        write("module " + node.name);
        if (node.body) {
            output << " {";
            indentLevel++;
            newLine();
            node.body->accept(*this);
            if (needsSemicolon) output << ";";
            indentLevel--;
            newLine();
            write("}");
        }
    }

    void visit(ErrorHandlingNode& node) override {
        switch (node.errorType) {
            case ErrorHandlingNode::ErrorType::Optional:
                node.expression->accept(*this);
                output << "?";
                break;
            case ErrorHandlingNode::ErrorType::Result:
                node.expression->accept(*this);
                output << "!";
                break;
            case ErrorHandlingNode::ErrorType::Matching:
                node.expression->accept(*this);
                if (node.errorHandler) {
                    output << " match ";
                    node.errorHandler.value()->accept(*this);
                }
                break;
        }
    }

    void visit(MatchNode& node) override {
        write("match ");
        node.matchExpression->accept(*this);
        output << " {";
        indentLevel++;
        for (auto& [pattern, stmt] : node.matchCases) {
            newLine();
            write("case ");
            pattern->accept(*this);
            output << ": ";
            stmt->accept(*this);
        }
        if (node.defaultCase) {
            newLine();
            write("default: ");
            node.defaultCase.value()->accept(*this);
        }
        indentLevel--;
        newLine();
        write("}");
    }

    void visit(StreamProcessingNode& node) override {
        write("stream ");
        node.inputStream->accept(*this);
        output << " -> ";
        node.processingFunction->accept(*this);
        output << " -> ";
        node.outputChannel->accept(*this);
    }

    void visit(AtomicNode& node) override {
        write("atomic ");
        switch (node.op) {
            case AtomicNode::Operation::FetchAdd:
                write("fetch_add(");
                node.target->accept(*this);
                write(", ");
                if (node.value) {
                    node.value.value()->accept(*this);
                }
                write(")");
                break;
            case AtomicNode::Operation::CompareExchange:
                write("compare_exchange(");
                node.target->accept(*this);
                write(", ");
                if (node.value) {
                    node.value.value()->accept(*this);
                }
                write(")");
                break;
            case AtomicNode::Operation::Load:
                write("load(");
                node.target->accept(*this);
                write(")");
                break;
            case AtomicNode::Operation::Store:
                write("store(");
                node.target->accept(*this);
                write(", ");
                if (node.value) {
                    node.value.value()->accept(*this);
                }
                write(")");
                break;
        }
    }

    void visit(ChannelNode& node) override {
        switch (node.op) {
            case ChannelNode::ChannelOperation::Send:
                write("send(");
                node.channel->accept(*this);
                if (node.data) {
                    output << ", ";
                    node.data.value()->accept(*this);
                }
                output << ")";
                break;
            case ChannelNode::ChannelOperation::Receive:
                write("receive(");
                node.channel->accept(*this);
                output << ")";
                break;
            case ChannelNode::ChannelOperation::Collect:
                write("collect(");
                node.channel->accept(*this);
                if (node.data) {
                    output << ", ";
                    node.data.value()->accept(*this);
                }
                output << ")";
                break;
        }
    }

    void visit(FieldNode& node) override {
        write(node.name + ": " + node.type.toString());
        if (node.initialValue) {
            output << " = ";
            // TODO: Need to handle Value type serialization
            output << "/* initial value */";
        }
    }

    void visit(LambdaNode& node) override {
        output << "|";
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            if (i > 0) output << ", ";
            output << node.parameters[i].name;
            if (node.parameters[i].type.tag != TypeTag::Any) {
                output << ": " << node.parameters[i].type.toString();
            }
        }
        output << "| ";
        node.body->accept(*this);
    }

    void visit(ImportNode& node) override {
        write("import " + node.moduleName);
        if (!node.alias->empty()) {
            output << " as " << node.alias.value();
        }
    }

    void visit(InterfaceNode& node) override {
        write("interface " + node.name);
        if (!node.name.empty()) {
            output << " extends " << node.name;
        }
        output << " {";
        indentLevel++;
        for (auto& method : node.methods) {
            newLine();
            method->accept(*this);
            output << ";";
        }
        indentLevel--;
        newLine();
        write("}");
    }

    void visit(MixinNode& node) override {
        write("mixin " + node.name);
        if (!node.methods.empty()) {
            output << " {";
            indentLevel++;
            for (auto& method : node.methods) {
                newLine();
                method->accept(*this);
            }
            indentLevel--;
            newLine();
            write("}");
        }
    }

    void visit(UnsafeNode& node) override {
        write("unsafe {");
        indentLevel++;
        for (auto& stmt : node.body) {
            newLine();
            stmt->accept(*this);
            if (needsSemicolon) output << ";";
        }
        indentLevel--;
        newLine();
        write("}");
    }

    void visit(InterpolatedStringNode& node) override {
        output << "`";
        for (const auto& part : node.parts) {
            // Since parts contains unique_ptr<ASTNode>, we need to check the actual type
            if (auto* stringLiteral = dynamic_cast<StringLiteralNode*>(part.get())) {
                // Handle string literal parts - output the string content directly
                if (std::holds_alternative<std::string>(stringLiteral->value.data)) {
                    output << std::get<std::string>(stringLiteral->value.data);
                }
            } else {
                // Handle expression parts - wrap in ${}
                output << "${";
                part->accept(*this);
                output << "}";
            }
        }
        output << "`";
    }

    void visit(BooleanNode& node) override {
        write(node.value ? "true" : "false");
    }

    void visit(UnaryNode& node) override {
        output << "(" << node.operatorType << " ";
        node.operand->accept(*this);
        output << ")";
    }

    void visit(CallNode& node) {
        node.accept(*this);
        output << "(";
        for (size_t i = 0; i < node.arguments.size(); ++i) {
            if (i > 0) output << ", ";
            node.arguments[i]->accept(*this);
        }
        output << ")";
    }

    void visit(GroupingNode& node) override {
        output << "(";
        node.expression->accept(*this);
        output << ")";
    }

    void visit(NilNode& node) override {
        write("nil");
    }
};
