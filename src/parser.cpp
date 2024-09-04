#include "parser.hh"
#include "instructions.hh"
#include "parser/packrat.hh"
#include "scanner.hh"
#include <chrono>
#include <fstream>
#include <thread>

Parser::Parser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem)
    : bytecode(Bytecode{})
    , scanner(scanner)
    , algo(Packrat{scanner, typeSystem})
    , variable(typeSystem)
{
    tokens = scanner.scanTokens();
    parse();
}

Bytecode Parser::parse()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    scanner.current = 0;
    PackratParser algo(scanner, typeSystem);
    algo.parse();
    //    while (!isAtEnd()) {
    //        PackratParser parser(scanner, typeSystem);
    //        parser.parse();
    //    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Parsing completed in " << duration.count() << " microseconds." << std::endl;
    return algo.getBytecode();
}

std::string Parser::toString() const
{
    return algo.toString();
}

std::vector<Instruction> Parser::getBytecode() const
{
    return algo.getBytecode();
}
