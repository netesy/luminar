//#ifndef CODEGEN_HH
//#define CODEGEN_HH

//#include "backend.hh"
//#include <vector>
//#include <string>
//#include <fstream>

//class CodegenBackend : public Backend {
//public:
//    explicit CodegenBackend(const std::string &outputFile);
//    ~CodegenBackend() override;

//    void execute(const Instruction &instruction) override;
//    void dumpRegisters() override;
//    void run(const std::vector<Instruction> &program) override;

//private:
//    std::ofstream outFile;
//    std::vector<Value> constants;   // Constants for storing data
//    std::vector<Value> variables;   // Variables for storing data

//    void handleLoadConst(const Value &constantValue);
//    void handleDeclareVariable(unsigned int variableIndex);
//    void handleLoadVariable(unsigned int variableIndex);
//    void handleStoreVariable(unsigned int variableIndex);
//    void handlePrint();
//    void handleHalt();
//    void handleBinaryOperation(const Instruction &instruction);
//    void handleUnaryOperation(const Instruction &instruction);
//    void handleJump(int targetAddress);
//    void handleJumpZero(int targetAddress);
//    void writeHeader();
//    void writeFooter();
//};

//#endif // CODEGEN_HH
