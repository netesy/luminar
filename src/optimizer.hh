// BytecodeOptimizer.hh
#pragma once

#include "Instructions.hh" // Assumes you have a struct for Instruction with Opcode and Value
#include "opcodes.hh"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using Bytecode = std::vector<Instruction>;

class BytecodeOptimizer
{
public:
    static Bytecode optimize(const Bytecode &bytecode)
    {
        Bytecode optimizedBytecode = bytecode;

        bool optimized = true;
        while (optimized) {
            optimized = false;
            optimized |= deadCodeElimination(optimizedBytecode);
            optimized |= constantFolding(optimizedBytecode);
            // optimized |= constantPropagation(optimizedBytecode);
            // optimized |= strengthReduction(optimizedBytecode);
            // optimized |= loopUnrolling(optimizedBytecode);
            // optimized |= redundantLoadStoreRemoval(optimizedBytecode);
            //  optimized |= inlineSimpleFunctions(optimizedBytecode);
            //  optimized |= tailCallOptimization(optimizedBytecode);
            optimized |= peepholeOptimization(optimizedBytecode);
        }

        return optimizedBytecode;
    }

private:
    // 1. Dead Code Elimination
    static bool deadCodeElimination(Bytecode &bytecode)
    {
        bool changesMade = false;
        std::vector<bool> reachable(bytecode.size(), false);
        std::unordered_set<std::string> calledFunctions;
        std::vector<size_t> stack = {0}; // Start with the first instruction

        while (!stack.empty()) {
            size_t current = stack.back();
            stack.pop_back();
            if (current >= bytecode.size() || reachable[current])
                continue;
            reachable[current] = true;

            switch (bytecode[current].opcode) {
            case JUMP:
            case JUMP_IF_FALSE:
            case JUMP_IF_TRUE: {
                int64_t offset = std::get<int64_t>(bytecode[current].value->data);
                stack.push_back(current + offset);
                if (bytecode[current].opcode != JUMP)
                    stack.push_back(current + 1);
            } break;
            case DEFINE_FUNCTION:
            case INVOKE_FUNCTION: {
                std::string funcName = std::get<std::string>(bytecode[current].value->data);
                calledFunctions.insert(funcName);
            }
            case RETURN:
            case PRINT:
            case LOAD_VARIABLE:
            case STORE_VARIABLE:
                stack.push_back(current + 1);
                break;
            default:
                stack.push_back(current + 1);
                break;
            }
        }

        Bytecode newBytecode;
        for (size_t i = 0; i < bytecode.size(); ++i) {
            if (reachable[i])
                newBytecode.push_back(bytecode[i]);
            else
                changesMade = true;
        }

        bytecode = std::move(newBytecode);
        return changesMade;
    }

    // 2. Constant Folding
    static bool constantFolding(Bytecode &bytecode)
    {
        bool changesMade = false;
        for (size_t i = 0; i < bytecode.size() - 2; ++i) {
            if (bytecode[i].opcode == LOAD_CONST && bytecode[i + 1].opcode == LOAD_CONST) {
                auto val1 = std::get<int64_t>(bytecode[i].value->data);
                auto val2 = std::get<int64_t>(bytecode[i + 1].value->data);

                switch (bytecode[i + 2].opcode) {
                case ADD:
                    bytecode[i].value->data = val1 + val2;
                    break;
                case SUBTRACT:
                    bytecode[i].value->data = val1 - val2;
                    break;
                case MULTIPLY:
                    bytecode[i].value->data = val1 * val2;
                    break;
                case DIVIDE:
                    if (val2 != 0)
                        bytecode[i].value->data = val1 / val2;
                    break;
                default:
                    continue;
                }

                bytecode[i + 1].opcode = NOP;
                bytecode[i + 2].opcode = NOP;
                changesMade = true;
            }
        }
        return changesMade;
    }

    // 3. Constant Propagation
    static bool constantPropagation(Bytecode &bytecode)
    {
        bool changesMade = false;
        std::unordered_map<std::string, int64_t> constants;

        for (size_t i = 0; i < bytecode.size(); ++i) {
            if (bytecode[i].opcode == LOAD_CONST) {
                std::string varName = std::get<std::string>(bytecode[i].value->data);
                int64_t constValue = std::get<int64_t>(bytecode[i + 1].value->data);
                constants[varName] = constValue;
                bytecode[i].opcode = NOP; // Mark as unused
                changesMade = true;
            } else if (bytecode[i].opcode == LOAD_VARIABLE) {
                std::string varName = std::get<std::string>(bytecode[i].value->data);
                if (constants.find(varName) != constants.end()) {
                    bytecode[i].opcode = LOAD_CONST; // Replace with constant load
                    bytecode[i].value->data = constants[varName];
                    changesMade = true;
                }
            }
        }

        // Clean up NOP instructions
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
        // for (auto &instr : bytecode) {
        //     if (instr.opcode == MULTIPLY || instr.opcode == DIVIDE) {
        //         int64_t constant = std::get<int64_t>(instr.value->data);
        //         if ((instr.opcode == MULTIPLY && constant == 2)
        //             || (instr.opcode == DIVIDE && constant == 2)) {
        //             instr.opcode = (instr.opcode == MULTIPLY) ? Opcode::ADD : Opcode::SHIFT_RIGHT;
        //             changesMade = true;
        //         }
        //     }
        // }
        return changesMade;
    }

    // 5. Loop Unrolling
    static bool loopUnrolling(Bytecode &bytecode)
    {
        bool changesMade = false;
        for (size_t i = 0; i < bytecode.size(); ++i) {
            if (bytecode[i].opcode == FOR_LOOP || bytecode[i].opcode == WHILE_LOOP) {
                size_t loopStart = i;
                // Attempt to unroll the loop if possible
                // This would involve checking the loop's body and replicating it
                // Example: If loop iterates a known fixed number of times
                // Here, you would implement the logic to unroll the loop
                // For simplicity, we assume unrolling by a factor of 2
                // You need to implement the actual logic based on your IR structure

                // Pseudo code example
                // for (int j = 0; j < 2; ++j) {
                //     copy the loop body here
                // }

                changesMade = true; // Set this to true if unrolling occurs
            }
        }
        return changesMade;
    }

    // 6. Redundant Load/Store Removal
    static bool redundantLoadStoreRemoval(Bytecode &bytecode)
    {
        bool changesMade = false;
        for (size_t i = 0; i < bytecode.size() - 1; ++i) {
            if (bytecode[i].opcode == LOAD_VARIABLE && bytecode[i + 1].opcode == STORE_VARIABLE
                && bytecode[i].value == bytecode[i + 1].value) {
                bytecode[i].opcode = NOP;
                bytecode[i + 1].opcode = NOP;
                changesMade = true;
            }
        }
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
                // Call the constant folding method here if it applies
                //changesMade |= constantFolding(bytecode, i);
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
                int64_t jumpOffset = std::get<int64_t>(bytecode[i].value->data);
                size_t targetIndex = i + jumpOffset;

                if (targetIndex < bytecode.size() && bytecode[targetIndex].opcode == NOP) {
                    // Replace jump to NOP with the next instruction
                    bytecode[i].opcode = NOP;
                    changesMade = true;
                }
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
