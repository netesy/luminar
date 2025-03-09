// #pragma once
// #include <libgccjit++.h>
// #include <memory>
// #include <unordered_map>
// #include "../function.hh"
// #include "../value.hh"
// #include "instruction.hh"

// class JitBackend {
// private:
//     gccjit::context ctx;
//     std::vector<Instruction>& program;
//     Functions& functions;
//     std::unordered_map<std::string, void*> compiledFunctions;
//     MemoryManager<> memoryManager;
//     std::stack<MemoryManager<>::Region*> regionStack;
//     std::stack<MemoryManager<>::Ref<Value>> stack;
//     std::vector<MemoryManager<>::Ref<Value>> variables;
//     size_t pc = 0;

//     struct JitTypes {
//         gccjit::type* valueType;
//         gccjit::type* valuePtrType;
//         gccjit::type* refValueType;
//         gccjit::type* regionType;
//         gccjit::type* regionPtrType;
//         gccjit::struct_type* memoryManagerType;
//     } jitTypes;

// public:
//     JitBackend(std::vector<Instruction>& program, Functions& funcs)
//         : program(program)
//         , functions(funcs)
//         , memoryManager(true)  // Enable statistics
//     {
//         ctx.set_bool_option(GCC_JIT_BOOL_OPTION_DEBUGINFO, 0);
//         ctx.set_int_option(GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 3);
//         initializeTypes();
//         initializeGlobalRegion();
//     }

//     ~JitBackend() {
//         clearStack();
//         memoryManager.printStatistics();
//         while (!regionStack.empty()) {
//             popRegion();
//         }
//     }

//     void pushRegion() {
//         regionStack.push(new MemoryManager<>::Region(memoryManager));
//     }

//     void popRegion() {
//         if (regionStack.size() > 1) {  // Keep global region
//             try {
//                 delete regionStack.top();
//                 regionStack.pop();
//             } catch (const std::exception& e) {
//                 std::cerr << "Error during region cleanup: " << e.what() << std::endl;
//             }
//         }
//     }

//     MemoryManager<>::Region& currentRegion() {
//         return *regionStack.top();
//     }

//     void push(const ValuePtr& valuePtr) {
//         auto refValue = memoryManager.makeRef<Value>(currentRegion(), *valuePtr);
//         stack.push(refValue);
//     }

//     ValuePtr pop() {
//         if (stack.empty()) {
//             std::cerr << "Error: Stack underflow" << std::endl;
//             return nullptr;
//         }
//         auto refValue = stack.top();
//         stack.pop();
//         return std::make_shared<Value>(*refValue);
//     }

//     void clearStack() {
//         std::cout << "Clearing stack" << std::endl;
//         while (!stack.empty()) {
//             try {
//                 stack.pop();
//             } catch (const std::exception& e) {
//                 std::cerr << "Error clearing stack: " << e.what() << std::endl;
//             }
//         }
//     }

//     void run() {
//         try {
//             auto start_time = std::chrono::high_resolution_clock::now();
//             compileProgram();
//             executeCompiledProgram();
//             auto end_time = std::chrono::high_resolution_clock::now();
//             auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
//             std::cout << "JIT Execution completed in " << duration.count() << " microseconds." << std::endl;
//         } catch (const std::exception& ex) {
//             std::cerr << "JIT execution error: " << ex.what() << std::endl;
//         }
//     }

// private:
//     void compileProgram() {
//         // Create main function that will contain our program
//         std::vector<gccjit::param> params;
//         auto mainFunc = ctx.new_function(GCC_JIT_FUNCTION_EXPORTED,
//                                          jitTypes.voidType,
//                                          "main_program",
//                                          params,
//                                          0);

//         auto block = mainFunc.new_block();

//         // Compile each instruction
//         for (const auto& inst : program) {
//             compileInstruction(block, inst);
//         }

//         // Add return statement
//         block.end_with_return();

//         // Compile the program
//         auto result = ctx.compile();
//         if (result) {
//             compiledFunctions["main_program"] = result.get_function("main_program");
//         } else {
//             throw std::runtime_error("Failed to compile program");
//         }
//     }

//     void compileInstruction(gccjit::block& block, const Instruction& inst) {
//         switch (inst.opcode) {
//         case LOAD_CONST:
//             compileLoadConst(block, inst);
//             break;

//         case ADD:
//         case SUBTRACT:
//         case MULTIPLY:
//         case DIVIDE:
//             compileBinaryOp(block, inst);
//             break;

//         case PRINT:
//             compilePrint(block);
//             break;

//         case DECLARE_VARIABLE:
//             compileDeclareVariable(block, inst);
//             break;

//         case STORE_VARIABLE:
//             compileStoreVariable(block, inst);
//             break;

//         case LOAD_VARIABLE:
//             compileLoadVariable(block, inst);
//             break;

//         default:
//             // For unsupported instructions, we'll add a call to the interpreter
//             compileInterpreterFallback(block, inst);
//         }
//     }

//     void compileLoadConst(gccjit::block& block, const Instruction& inst) {
//         // Create a new Value object and initialize it with the constant
//         auto valuePtr = block.new_call(ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.valuePtrType,
//             "createValue",
//             {}
//             ));

//         // Store the constant value
//         switch (inst.value->type->tag) {
//         case TypeTag::Int: {
//             auto intVal = std::get<int64_t>(inst.value->data);
//             block.add_assignment(
//                 block.new_array_access(valuePtr, ctx.new_rvalue(jitTypes.int64Type, intVal)),
//                 ctx.new_rvalue(jitTypes.int64Type, intVal)
//                 );
//             break;
//         }
//             // Add other type cases...
//         }

//         // Push to stack
//         block.add_eval(block.new_call(
//             ctx.new_function(
//                 GCC_JIT_FUNCTION_IMPORTED,
//                 jitTypes.voidType,
//                 "pushStack",
//                 {ctx.new_param(jitTypes.valuePtrType, "value")}
//                 ),
//             {valuePtr}
//             ));
//     }

//     void compileBinaryOp(gccjit::block& block, const Instruction& inst) {
//         // Pop operands from stack
//         auto val2 = block.new_call(ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.valuePtrType,
//             "popStack",
//             {}
//             ));

//         auto val1 = block.new_call(ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.valuePtrType,
//             "popStack",
//             {}
//             ));

//         // Perform operation based on type
//         auto result = block.new_call(ctx.new_function(
//                                          GCC_JIT_FUNCTION_IMPORTED,
//                                          jitTypes.valuePtrType,
//                                          getOperationFunction(inst.opcode),
//                                          {
//                                              ctx.new_param(jitTypes.valuePtrType, "val1"),
//                                              ctx.new_param(jitTypes.valuePtrType, "val2")
//                                          }
//                                          ), {val1, val2});

//         // Push result to stack
//         block.add_eval(block.new_call(
//             ctx.new_function(
//                 GCC_JIT_FUNCTION_IMPORTED,
//                 jitTypes.voidType,
//                 "pushStack",
//                 {ctx.new_param(jitTypes.valuePtrType, "value")}
//                 ),
//             {result}
//             ));
//     }

//     const char* getOperationFunction(Opcode op) {
//         switch (op) {
//         case ADD: return "addValues";
//         case SUBTRACT: return "subtractValues";
//         case MULTIPLY: return "multiplyValues";
//         case DIVIDE: return "divideValues";
//         default: return "unknownOp";
//         }
//     }

//     void compilePrint(gccjit::block& block) {
//         // Pop value from stack
//         auto value = block.new_call(ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.valuePtrType,
//             "popStack",
//             {}
//             ));

//         // Call print function
//         block.add_eval(block.new_call(
//             ctx.new_function(
//                 GCC_JIT_FUNCTION_IMPORTED,
//                 jitTypes.voidType,
//                 "printValue",
//                 {ctx.new_param(jitTypes.valuePtrType, "value")}
//                 ),
//             {value}
//             ));
//     }

//     void executeCompiledProgram() {
//         auto mainFn = reinterpret_cast<void(*)()>(compiledFunctions["main_program"]);
//         if (mainFn) {
//             mainFn();
//         } else {
//             throw std::runtime_error("Failed to execute compiled program");
//         }
//     }

//     void initializeGlobalRegion() {
//         regionStack.push(new MemoryManager<>::Region(memoryManager));
//     }

//     void initializeTypes() {
//         // Define JIT types that correspond to our C++ types
//         jitTypes.valueType = ctx.new_opaque_struct("Value");
//         jitTypes.valuePtrType = ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.valueType);
//         jitTypes.refValueType = ctx.new_opaque_struct("RefValue");
//         jitTypes.regionType = ctx.new_opaque_struct("Region");
//         jitTypes.regionPtrType = ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.regionType);

//         // Define MemoryManager type
//         auto memoryManagerFields = std::vector<gccjit::field>{
//             ctx.new_field(jitTypes.regionPtrType, "currentRegion")
//         };
//         jitTypes.memoryManagerType = ctx.new_struct_type("MemoryManager", memoryManagerFields);
//     }

//     void compileMemoryOperations(gccjit::function& func) {
//         // Create runtime functions for memory operations
//         auto pushRegionFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             ctx.get_type(GCC_JIT_TYPE_VOID),
//             "pushRegion",
//             {
//                 ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager")
//             }
//             );

//         auto popRegionFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             ctx.get_type(GCC_JIT_TYPE_VOID),
//             "popRegion",
//             {
//                 ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager")
//             }
//             );

//         auto makeRefFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.refValueType,
//             "makeRef",
//             {
//                 ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager"),
//                 ctx.new_param(jitTypes.regionPtrType, "region"),
//                 ctx.new_param(jitTypes.valuePtrType, "value")
//             }
//             );
//     }

//     void compileVariableOperations() {
//         // Compile variable declaration
//         auto declareVarFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             ctx.get_type(GCC_JIT_TYPE_VOID),
//             "declareVariable",
//             {
//                 ctx.new_param(ctx.get_type(GCC_JIT_TYPE_INT), "index"),
//                 ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager"),
//                 ctx.new_param(jitTypes.regionPtrType, "region")
//             }
//             );

//         // Compile variable store
//         auto storeVarFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             ctx.get_type(GCC_JIT_TYPE_VOID),
//             "storeVariable",
//             {
//                 ctx.new_param(ctx.get_type(GCC_JIT_TYPE_INT), "index"),
//                 ctx.new_param(jitTypes.valuePtrType, "value"),
//                 ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager"),
//                 ctx.new_param(jitTypes.regionPtrType, "region")
//             }
//             );

//         // Compile variable load
//         auto loadVarFunc = ctx.new_function(
//             GCC_JIT_FUNCTION_IMPORTED,
//             jitTypes.valuePtrType,
//             "loadVariable",
//             {
//                 ctx.new_param(ctx.get_type(GCC_JIT_TYPE_INT), "index")
//             }
//             );
//     }

//     void compileVariableInstruction(gccjit::block& block, const Instruction& inst) {
//         switch (inst.opcode) {
//         case DECLARE_VARIABLE: {
//             auto index = std::get<int32_t>(inst.value->data);
//             block.add_eval(block.new_call(
//                 ctx.new_function(
//                     GCC_JIT_FUNCTION_IMPORTED,
//                     ctx.get_type(GCC_JIT_TYPE_VOID),
//                     "declareVariable",
//                     {
//                         ctx.new_param(ctx.get_type(GCC_JIT_TYPE_INT), "index"),
//                         ctx.new_param(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), "memoryManager"),
//                         ctx.new_param(jitTypes.regionPtrType, "region")
//                     }
//                     ),
//                 {
//                     ctx.new_rvalue(ctx.get_type(GCC_JIT_TYPE_INT), index),
//                     ctx.new_rvalue(ctx.new_type(GCC_JIT_TYPE_POINTER, jitTypes.memoryManagerType), &memoryManager),
//                     ctx.new_rvalue(jitTypes.regionPtrType, &currentRegion())
//                 }
//                 ));
//             break;
//         }
//         case STORE_VARIABLE: {
//             auto index = std::get<int32_t>(inst.value->data);
//             auto value = pop();
//             if (index >= static_cast<int32_t>(variables.size())) {
//                 variables.resize(index + 1);
//             }
//             variables[index] = memoryManager.makeRef<Value>(currentRegion(), *value);
//             break;
//         }
//         case LOAD_VARIABLE: {
//             auto index = std::get<int32_t>(inst.value->data);
//             if (index >= static_cast<int32_t>(variables.size())) {
//                 throw std::runtime_error("Invalid variable index");
//             }
//             push(std::make_shared<Value>(*variables[index]));
//             break;
//         }
//         }
//     }

//     // Helper runtime functions that will be called from JIT-compiled code
//     static ValuePtr createValue() {
//         return std::make_shared<Value>();
//     }

//     static void pushStack(ValuePtr value) {
//         // Implementation to push to stack
//     }

//     static ValuePtr popStack() {
//         // Implementation to pop from stack
//         return nullptr;
//     }

//     static void printValue(ValuePtr value) {
//         // Implementation to print value
//     }
// };
