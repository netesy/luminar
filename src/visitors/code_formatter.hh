// // src/visitors/code_formatter.hh
// #pragma once

// #include "../ast.hh"
// #include <string>
// #include <vector>
// #include <sstream>

// class CodeFormatter : public ASTVisitor {
// public:
//     virtual ~CodeFormatter() = default;
//     // virtual std::string format(ASTNode& node) = 0;

// protected:
//     std::ostringstream output;
//     std::shared_ptr<TypeSystem> typeSystem;
//     int indentLevel = 0;
//     bool atLineStart = true;
//     bool needsSemicolon = true;

//     void addIndent() {
//         if (atLineStart) {
//             output << std::string(indentLevel * 4, ' ');
//             atLineStart = false;
//         }
//     }

//     void newLine() {
//         output << '\n';
//         atLineStart = true;
//         needsSemicolon = true;
//     }

//     void write(const std::string& str) {
//         addIndent();
//         output << str;
//     }

// public:
//     explicit CodeFormatter(std::shared_ptr<TypeSystem> ts) : typeSystem(std::move(ts)) {}

//     std::string format(ASTNode& node) {
//         output.str("");  // Clear any existing content
//         output.clear();   // Clear any error flags
//         node.accept(*this);
//         return output.str();
//     }

//     void visit(NumberNode& node) override {
//         // Access the numeric value from the Value variant
//         if (std::holds_alternative<int32_t>(node.value.data)) {
//             output << std::get<int32_t>(node.value.data);
//         } else if (std::holds_alternative<int64_t>(node.value.data)) {
//             output << std::get<int64_t>(node.value.data);
//         } else if (std::holds_alternative<double>(node.value.data)) {
//             output << std::get<double>(node.value.data);
//         } else if (std::holds_alternative<float>(node.value.data)) {
//             output << std::get<float>(node.value.data);
//         } else {
//             output << "0"; // Default fallback
//         }
//     }

//     void visit(StringLiteralNode& node) override {
//         if (std::holds_alternative<std::string>(node.value.data)) {
//             output << "\"" << std::get<std::string>(node.value.data) << "\"";
//         } else {
//             output << "\"\""; // Empty string as fallback
//         }
//     }

//     void visit(BinaryNode& node) override {
//         output << "(";
//         node.left->accept(*this);
//         output << " " << node.operatorType << " ";
//         node.right->accept(*this);
//         output << ")";
//     }

//     void visit(BlockNode& node) override {
//         output << "{\n";
//         indentLevel++;

//         for (const auto& stmt : node.getStatements()) {
//             (*stmt).accept(*this);
//             if (needsSemicolon) {
//                 output << ";";
//             }
//             newLine();
//         }

//         indentLevel--;
//         addIndent();
//         output << "}";
//         needsSemicolon = false;
//     }

//     void visit(VariableNode& node) override {
//         write((node.isMutable() ? "var " : "const ") + node.getName());

//         // Get the variable's type
//         TypePtr varType = node.inferType(*typeSystem);

//         // Always show type if it's explicitly specified or can't be inferred
//         if (varType && varType->tag != TypeTag::Any) {
//             output << ": " << varType->toString();
//         }

//         if (node.hasInitializer()) {
//             output << " = ";
//             node.getInitializer()->accept(*this);

//             // Show inferred type if different from declared type
//             if (Expression* initExpr = node.getInitializer()) {
//                 TypePtr initType = initExpr->inferType(*typeSystem);
//                 if (initType && initType->tag != TypeTag::Any &&
//                     (!varType || initType->tag != varType->tag)) {
//                     output << " /* : " << initType->toString() << " */";
//                 }
//             }
//         }
//     }

//     void visit(ConditionalNode& node) override {
//         write("if (");
//         node.condition->accept(*this);
//         output << ") ";
//         node.thenBranch->accept(*this);

//         // Handle elif branches
//         for (const auto& elif : node.elifBranches) {
//             output << " elif (";
//             elif.first->accept(*this);
//             output << ") ";
//             elif.second->accept(*this);
//         }

//         // Handle else branch
//         if (node.elseBranch) {
//             output << " else ";
//             (*node.elseBranch)->accept(*this);
//         }
//         needsSemicolon = false;
//     }

//     void visit(WhileNode& node) override {
//         write("while (");
//         node.condition->accept(*this);
//         output << ") ";
//         node.body->accept(*this);
//         needsSemicolon = false;
//     }

//     void visit(RangeNode&) override {}
//     void visit(ListNode&) override {}
//     void visit(DictNode&) override {}
//     void visit(ParallelNode&) override {}
//     void visit(ConcurrentNode&) override {}
//     void visit(FunctionNode&) override {}
//     void visit(ClassNode&) override {}
//     void visit(ModuleNode&) override {}
//     void visit(ErrorHandlingNode&) override {}
//     void visit(MatchNode&) override {}
//     void visit(StreamProcessingNode&) override {}
//     void visit(AtomicNode&) override {}
//     void visit(ChannelNode&) override {}
//     void visit(FieldNode&) override {}
//     void visit(LambdaNode&) override {}
//     void visit(ImportNode&) override {}
//     void visit(InterfaceNode&) override {}
//     void visit(MixinNode&) override {}
//     void visit(UnsafeNode&) override {}
//     void visit(InterpolatedStringNode&) override {}
//     void visit(BooleanNode&) override {}
//     void visit(UnaryNode&) override {}
//     void visit(CallNode&) override {}
//     void visit(GroupingNode&) override {}
//     void visit(ReturnNode&) override {}
//     void visit(NilNode&) override {}
// };
