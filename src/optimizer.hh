// BytecodeOptimizer.hh
#pragma once

#include "instructions.hh" // Assumes you have a struct for Instruction with Opcode and Value
#include "opcodes.hh"
#include <chrono> // For timing
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using Bytecode = std::vector<Instruction>;

class BytecodeOptimizer
{
public:
    // Added debug parameter with default value of false
    static Bytecode optimize(const Bytecode &bytecode, bool debug = false)
    {
        if (bytecode.empty()) {
            debugLog("Empty bytecode received", debug);
            return bytecode;
        }

        Bytecode optimizedBytecode = bytecode;
        bool optimized = true;
        std::unordered_map<std::string, double> timingResults;

        const size_t MAX_ITERATIONS
            = std::max(size_t(10),               // Minimum iterations for very small bytecode
                       2 * (bytecode.size() / 2) // Each pass could optimize pairs of instructions
            );

        // Track optimization effectiveness
        size_t prevSize = optimizedBytecode.size();
        size_t unchangedCount = 0;
        const size_t STAGNATION_LIMIT = 4;

        debugLog("Initial bytecode size: " + std::to_string(bytecode.size()), debug);
        debugLog("Maximum iterations set to: " + std::to_string(MAX_ITERATIONS), debug);

        size_t iterationCount = 0;

        while (optimized && iterationCount++ < MAX_ITERATIONS) {
            optimized = false;

            // Measure time taken by each optimization pass
            timingResults["Dead Code Elimination"] += measureTime("Dead Code Elimination",
                                                                  optimizedBytecode,
                                                                  optimized,
                                                                  deadCodeElimination,
                                                                  debug);
            timingResults["Strength Reduction"] += measureTime("Strength Reduction",
                                                               optimizedBytecode,
                                                               optimized,
                                                               strengthReduction,
                                                               debug);
            timingResults["Constant Folding"] += measureTime("Constant Folding",
                                                             optimizedBytecode,
                                                             optimized,
                                                             constantFolding,
                                                             debug);
            timingResults["Constant Propagation"] += measureTime("Constant Propagation",
                                                                 optimizedBytecode,
                                                                 optimized,
                                                                 constantPropagation,
                                                                 debug);
            timingResults["Loop Unrolling"] += measureTime("Loop Unrolling",
                                                           optimizedBytecode,
                                                           optimized,
                                                           loopUnrolling,
                                                           debug);
            timingResults["Inline Simple Functions"] += measureTime("Inline Simple Functions",
                                                                    optimizedBytecode,
                                                                    optimized,
                                                                    inlineSimpleFunctions,
                                                                    debug);
            timingResults["Tail Call Optimization"] += measureTime("Tail Call Optimization",
                                                                   optimizedBytecode,
                                                                   optimized,
                                                                   tailCallOptimization,
                                                                   debug);
            timingResults["Peephole Optimization"] += measureTime("Peephole Optimization",
                                                                  optimizedBytecode,
                                                                  optimized,
                                                                  peepholeOptimization,
                                                                  debug);

            // Check if bytecode size has changed
            if (optimizedBytecode.size() == prevSize) {
                unchangedCount++;
                if (unchangedCount >= STAGNATION_LIMIT) {
                    debugLog("Optimization stagnated - no size reduction for "
                                 + std::to_string(STAGNATION_LIMIT) + " iterations",
                             debug);
                    break;
                }
            } else {
                unchangedCount = 0;
                prevSize = optimizedBytecode.size();
            }
        }

        // Report optimization results if debug is enabled
        if (debug) {
            double reductionPercent = 100.0
                                      * (1.0 - (double) optimizedBytecode.size() / bytecode.size());
            std::cout << "Optimization completed after " << iterationCount << " iterations\n"
                      << "Original size: " << bytecode.size()
                      << ", Final size: " << optimizedBytecode.size() << " (" << reductionPercent
                      << "% reduction)\n";

            if (iterationCount >= MAX_ITERATIONS) {
                std::cerr << "Warning - reached maximum iteration count (" << MAX_ITERATIONS
                          << ")\n";
            }

            // Output timing results
            std::cout << "Optimization Timings:\n";
            for (const auto &[name, duration] : timingResults) {
                std::cout << name << ": " << duration << " ms\n";
            }
        }

        return optimizedBytecode;
    }

private:
    std::shared_ptr<TypeSystem> typeSystem = std::make_shared<TypeSystem>();

    // Helper function for debug logging
    static void debugLog(const std::string &message, bool debug)
    {
        if (debug) {
            std::cout << "DEBUG: " << message << "\n";
        }
    }

    template<typename Func>
    static double measureTime(
        const std::string &name, Bytecode &bytecode, bool &optimized, Func func, bool debug)
    {
        auto start = std::chrono::high_resolution_clock::now();
        bool result = func(bytecode);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        // Only set optimized if the function actually made changes
        if (result) {
            debugLog(name + " made changes to bytecode", debug);
            optimized = true;
        }
        return elapsed.count();
    }

    static bool safeVariantAccess(const Value *value, TypeTag expectedType)
    {
        return value && value->type->tag == expectedType;
    }
    // 1. Dead Code Elimination
    static bool deadCodeElimination(Bytecode &bytecode)
    {
        bool changesMade = false;
        // std::vector<bool> reachable(bytecode.size(), false);
        // std::unordered_set<std::string> calledFunctions;
        // std::vector<size_t> stack = {0}; // Start with the first instruction

        // while (!stack.empty()) {
        //     size_t current = stack.back();
        //     stack.pop_back();
        //     if (current >= bytecode.size() || reachable[current])
        //         continue;
        //     reachable[current] = true;

        //     switch (bytecode[current].opcode) {
        //     case JUMP:
        //     case JUMP_IF_FALSE:
        //     case JUMP_IF_TRUE: {
        //         //var j: int = 0; while (j < 5) { print("While loop iteration: {j}"); j += 1;}
        //         // std::cout << bytecode[current].value->data << std::endl;
        //         // int64_t offset = std::get<int64_t>(bytecode[current].value->data);
        //         // size_t target = current + offset;

        //         // // Push the target of the jump to the stack
        //         // if (target < bytecode.size()) {
        //         //     stack.push_back(target);
        //         // }

        //         // // For JUMP_IF_* instructions, also push the next instruction
        //         // if (bytecode[current].opcode != Opcode::JUMP) {
        //         //     stack.push_back(current + 1); // Continue to next instruction
        //         // }
        //         // break;
        //         if (std::holds_alternative<int32_t>(bytecode[current].value->data)) {
        //             int32_t offset = std::get<int32_t>(bytecode[current].value->data);
        //             size_t target = current + offset;

        //             // Push the target of the jump to the stack
        //             if (target < bytecode.size()) {
        //                 stack.push_back(target);
        //             }

        //             // For JUMP_IF_* instructions, also push the next instruction
        //             if (bytecode[current].opcode != Opcode::JUMP) {
        //                 stack.push_back(current + 1); // Continue to next instruction
        //             }
        //         } else {
        //             // Handle unexpected types
        //             if (std::holds_alternative<std::string>(bytecode[current].value->data)) {
        //                 std::cerr
        //                     << "Error: Expected int64_t, but found std::string at instruction "
        //                     << current << std::endl;
        //             } else if (std::holds_alternative<bool>(bytecode[current].value->data)) {
        //                 std::cerr << "Error: Expected int64_t, but found bool at instruction "
        //                           << current << std::endl;
        //             } else {
        //                 std::cerr << "Error: Unexpected type at instruction " << current
        //                           << std::endl;
        //             }
        //             changesMade = true; // Mark as a change if needed
        //         }
        //         break;
        //     };
        //     case DEFINE_FUNCTION:
        //     case INVOKE_FUNCTION: {
        //         std::string funcName = std::get<std::string>(bytecode[current].value->data);
        //         calledFunctions.insert(funcName);
        //     }
        //     case RETURN:
        //     case PRINT:
        //     case LOAD_VARIABLE:
        //     case STORE_VARIABLE:
        //         stack.push_back(current + 1);
        //         break;
        //     default:
        //         stack.push_back(current + 1);
        //         break;
        //     }
        // }

        // Bytecode newBytecode;
        // for (size_t i = 0; i < bytecode.size(); ++i) {
        //     if (reachable[i])
        //         newBytecode.push_back(bytecode[i]);
        //     else
        //         changesMade = true;
        // }

        // bytecode = std::move(newBytecode);
        return changesMade;
    }

    // 2. Constant Folding
    template<typename T>
    static std::optional<T> performOperation(T val1, T val2, Opcode op)
    {
        switch (op) {
        case ADD:
            return val1 + val2;
        case SUBTRACT:
            return val1 - val2;
        case MULTIPLY:
            return val1 * val2;
        case DIVIDE:
            if (val2 != 0) {
                return val1 / val2;
            }
            return std::nullopt;

        case GREATER_THAN:
            return val1 > val2;
        case GREATER_THAN_OR_EQUAL:
            return val1 >= val2;
        case LESS_THAN:
            return val1 > val2;
        case LESS_THAN_OR_EQUAL:
            return val1 >= val2;
        case EQUAL:
            return val1 == val2;
        case NOT_EQUAL:
            return val1 != val2;

        default:
            return std::nullopt;
        }
    }

    static bool constantFolding(Bytecode &bytecode)
    {
        bool changesMade = false;
        std::shared_ptr<TypeSystem> typeSystem = std::make_shared<TypeSystem>();

        for (size_t i = 0; i < bytecode.size() - 2; ++i) {
            auto &first = bytecode[i];
            auto &second = bytecode[i + 1];
            auto &operation = bytecode[i + 2];

            // Skip if not two consecutive constants
            if (first.opcode != LOAD_CONST || second.opcode != LOAD_CONST) {
                continue;
            }

            // Get common type between the two values
            TypePtr commonType = typeSystem->getCommonType(first.value->type, second.value->type);
            bool foldedSuccessfully = false;

            // Handle integer operations
            if (commonType->tag == TypeTag::Int) {
                int64_t val1 = std::get<int64_t>(first.value->data);
                int64_t val2 = std::get<int64_t>(second.value->data);

                if (auto result = performOperation(val1, val2, operation.opcode)) {
                    first.value->data = *result;
                    first.value->type = commonType;
                    foldedSuccessfully = true;
                }
            }
            // Handle floating point operations
            else if (commonType->tag == TypeTag::Float64) {
                double val1 = std::get<double>(first.value->data);
                double val2 = std::get<double>(second.value->data);

                if (auto result = performOperation(val1, val2, operation.opcode)) {
                    first.value->data = *result;
                    first.value->type = commonType;
                    foldedSuccessfully = true;
                }
            }

            // If folding succeeded, mark the unused instructions as NOPs
            if (foldedSuccessfully) {
                second.opcode = NOP;
                operation.opcode = NOP;
                changesMade = true;
            }
        }

        return changesMade;
    }

    // 3. Constant Propagation
    static bool constantPropagation(Bytecode &bytecode)
    {
        bool changesMade = false;
        std::unordered_map<int64_t, std::optional<Value>> constantValues;

        // Original constant propagation logic
        // for (size_t i = 0; i < bytecode.size(); ++i) {
        //     auto &instruction = bytecode[i];
        //     if (instruction.opcode == Opcode::STORE_VARIABLE) {
        //         int64_t varName = std::get<int64_t>(instruction.value->data);
        //         if (instruction.value->type->tag == TypeTag::Int
        //             || instruction.value->type->tag == TypeTag::Float64
        //             || instruction.value->type->tag == TypeTag::Bool) {
        //             constantValues[varName] = *instruction.value;
        //             changesMade = true;
        //         } else {
        //             constantValues[varName] = std::nullopt;
        //         }
        //     } else if (instruction.opcode == Opcode::LOAD_VARIABLE) {
        //         int64_t varName = std::get<int64_t>(instruction.value->data);
        //         if (constantValues.count(varName) && constantValues[varName].has_value()) {
        //             Instruction newInstruction(Opcode::NOP, 1);
        //             newInstruction.opcode = Opcode::LOAD_CONST;
        //             newInstruction.value = std::make_shared<Value>(*constantValues[varName]);
        //             bytecode[i] = newInstruction;
        //             changesMade = true;
        //         }
        //     }
        // }

        // Clean up NOP instructions only
        bytecode.erase(std::remove_if(bytecode.begin(),
                                      bytecode.end(),
                                      [](const Instruction &instr) { return instr.opcode == NOP; }),
                       bytecode.end());

        return changesMade;
    }

    // 4. Strength Reduction
    static bool strengthReduction(Bytecode &bytecode)
    {
        bool changesMade = false;

        for (auto &instr : bytecode) {
            if (instr.opcode == MULTIPLY || instr.opcode == DIVIDE) {
                if (safeVariantAccess(instr.value.get(), TypeTag::Int)) { // Check it's an int64_t
                    int64_t constant = std::get<int64_t>(instr.value->data);

                    if (constant == 2) {
                        if (instr.opcode == MULTIPLY) {
                            instr.opcode = Opcode::ADD; // Replace MULTIPLY with ADD
                            int64_t currentValue = std::get<int64_t>(instr.value->data);
                            instr.value->data = currentValue; // Just add the same value twice later
                        } else if (instr.opcode == DIVIDE) {
                            int64_t currentValue = std::get<int64_t>(instr.value->data);
                            instr.value->data = currentValue >> 1; // Perform right shift by 1
                        }
                        changesMade = true;
                    }
                }
            }
        }
        return changesMade;
    }

    // 5. Loop Unrolling
    static bool loopUnrolling(Bytecode &bytecode)
    {
        bool changesMade = false;
        // for (size_t i = 0; i < bytecode.size(); ++i) {
        //     if (bytecode[i].opcode == FOR_LOOP || bytecode[i].opcode == WHILE_LOOP) {
        //         size_t loopStart = i;
        //         // Attempt to unroll the loop if possible
        //         // This would involve checking the loop's body and replicating it
        //         // Example: If loop iterates a known fixed number of times
        //         // Here, you would implement the logic to unroll the loop
        //         // For simplicity, we assume unrolling by a factor of 2
        //         // You need to implement the actual logic based on your IR structure

        //         // Pseudo code example
        //         // for (int j = 0; j < 2; ++j) {
        //         //     copy the loop body here
        //         // }

        //         changesMade = true; // Set this to true if unrolling occurs
        //     }
        // }
        return changesMade;
    }

    // 7. Inline Simple Functions
    static bool inlineSimpleFunctions(Bytecode &bytecode)
    {
        bool changesMade = false;
        // Implementation here would detect small function bodies that can be inlined
        return changesMade;
    }

    // 8. Tail Call Optimization
    static bool tailCallOptimization(Bytecode &bytecode)
    {
        bool changesMade = false;
        for (size_t i = 0; i < bytecode.size() - 1; ++i) {
            if (bytecode[i].opcode == INVOKE_FUNCTION && bytecode[i + 1].opcode == RETURN) {
                bytecode[i + 1].opcode = NOP; // Eliminate the redundant return
                changesMade = true;
            }
        }
        return changesMade;
    }

    // 9. Peephole Optimization
    static bool peepholeOptimization(Bytecode &bytecode)
    {
        bool changesMade = false;

        for (size_t i = 0; i < bytecode.size(); ++i) {
            // Optimization 1: Eliminate redundant NOPs
            if (bytecode[i].opcode == NOP) {
                bytecode.erase(bytecode.begin() + i);
                changesMade = true;
                --i; // Adjust index after removal
                continue;
            }

            // Optimization 2: Remove redundant LOAD followed by STORE
            if (i < bytecode.size() - 1 && bytecode[i].opcode == LOAD_VARIABLE
                && bytecode[i + 1].opcode == STORE_VARIABLE
                && bytecode[i].value == bytecode[i + 1].value) {
                bytecode[i].opcode = NOP; // Remove redundant load/store
                bytecode[i + 1].opcode = NOP;
                changesMade = true;
            }

            // Optimization 3: Perform constant folding for arithmetic operations
            if (bytecode[i].opcode == LOAD_CONST) {
                changesMade |= constantFolding(bytecode);
            }

            // Optimization 4: Eliminate unreachable code (NOPs)
            if (bytecode[i].opcode == HALT) {
                // Remove any instructions after HALT
                bytecode.erase(bytecode.begin() + i + 1, bytecode.end());
                changesMade = true;
                break; // Exit after HALT, no need to check further
            }

            // Optimization 5: Simplify control flow with JUMP and JUMP_IF
            if (i < bytecode.size() - 1
                && (bytecode[i].opcode == JUMP || bytecode[i].opcode == JUMP_IF_TRUE
                    || bytecode[i].opcode == JUMP_IF_FALSE)) {
                int64_t jumpOffset = std::get<int32_t>(bytecode[i].value->data);
                size_t targetIndex = i + jumpOffset;

                if (targetIndex < bytecode.size() && bytecode[targetIndex].opcode == NOP) {
                    // Replace jump to NOP with the next instruction
                    bytecode[i].opcode = NOP;
                    changesMade = true;
                }
            }
            // // JUMP Simplification
            // if (bytecode[i].opcode == Opcode::JUMP && i + 1 < bytecode.size()) {
            //     // Check if there's another jump at the target offset
            //     int jumpOffset = std::get<int32_t>(
            //         bytecode[i + 1].value->data); // Assume offset is next in bytecode
            //     int targetIndex = i + jumpOffset;

            //     if (targetIndex >= 0 && targetIndex < bytecode.size()
            //         && bytecode[targetIndex].opcode == Opcode::JUMP) {
            //         // Simplify by jumping to the final target
            //         int finalJumpOffset = std::get<int32_t>(bytecode[targetIndex + 1].value->data);
            //         bytecode[i + 1].value->data
            //             = jumpOffset
            //               + finalJumpOffset; // Update the first jump to reach the final target
            //         changesMade = true;
            //     }

            //     // Remove unreachable code between jumps
            //     for (size_t j = i + 2; j < i + 1 + jumpOffset && j < bytecode.size(); ++j) {
            //         bytecode[j].opcode = Opcode::NOP; // Convert unreachable code to NOPs
            //     }
            // }

            // Remove redundant consecutive LOAD_STR or LOAD_CONST followed by LOAD_VALUE
            // if (i < bytecode.size() - 2
            //     && (bytecode[i].opcode == LOAD_STR || bytecode[i].opcode == LOAD_CONST)
            //     && (bytecode[i+1].opcode == LOAD_STR || bytecode[i+1].opcode == LOAD_CONST)
            //     && bytecode[i+2].opcode == LOAD_VALUE) {

            //     // Remove the  LOAD
            //     bytecode[i].opcode = NOP;
            //     bytecode[i+1].opcode = NOP;
            //     changesMade = true;
            // } this removes only one if there are multiple

            if (i < bytecode.size() - 2
                && (bytecode[i].opcode == LOAD_STR || bytecode[i].opcode == LOAD_CONST)
                && bytecode[i+1].opcode == LOAD_VALUE) {

                // Remove the  LOAD
                bytecode[i].opcode = NOP;
                changesMade = true;
            }

            // Boolean Strength Reduction (a && true -> a)
            if (i > 1 && bytecode[i].opcode == Opcode::LOAD_CONST
                && bytecode[i - 1].opcode == Opcode::AND
                && bytecode[i + 1].opcode == Opcode::LOAD_CONST
                && bytecode[i + 1].opcode == Opcode::BOOLEAN) {
                // Replace `AND true` with a NOP to simplify the expression.
                bytecode[i - 1].opcode = Opcode::NOP;
                bytecode[i].opcode = Opcode::NOP;
                changesMade = true;
            }
            // Optimization 6: Replace multiple STORE_VARIABLE with a single store
            if (i < bytecode.size() - 1 && bytecode[i].opcode == STORE_VARIABLE
                && bytecode[i + 1].opcode == STORE_VARIABLE
                && bytecode[i].value == bytecode[i + 1].value) {
                bytecode[i + 1].opcode = NOP; // Remove the second STORE
                changesMade = true;
            }
        }

        // Clean up NOP instructions
        bytecode.erase(std::remove_if(bytecode.begin(),
                                      bytecode.end(),
                                      [](const Instruction &instr) { return instr.opcode == NOP; }),
                       bytecode.end());

        return changesMade;
    }
};
