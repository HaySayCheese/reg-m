#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>


typedef unsigned long long int RegValue;
typedef std::size_t InstructionPos;
typedef std::size_t RegNumber;

class Instruction
{
public:
    enum Type {
        CT_Z = 1, CT_S, CT_T, CT_J
    };

    Instruction(Type type, RegNumber arg1=0, RegNumber arg2=0, InstructionPos instr=0);
    void setType(Type type);
    Type type() const;

public:
    RegNumber arg1;
    RegNumber arg2;
    RegValue  instr;

private:
    Type mType;
};


//-- instructions exceptions
class CommandException: public std::runtime_error
{
public:
    CommandException(std::string message, std::size_t errorIndex):
        runtime_error(message), mIndex(errorIndex){}

    std::size_t index() const {
        return mIndex;
    }

private:
    std::size_t mIndex;
};

class EmptyCommandExpcept: public std::exception
{
};
class InvalidCommandSyntaxExcept: public CommandException
{
public:
    InvalidCommandSyntaxExcept(std::string message, std::size_t errorIndex):
        CommandException(message, errorIndex){}
};


//-- interpreter
class Interpreter
{
public:
    Interpreter();

    bool parseFile(std::string fileName);

private:
    bool parseLine(std::string command);
    bool parseInstruction(std::string instr, Instruction::Type commandType, std::size_t carretOffset);
    bool parseInitInstruction(std::string instr, std::size_t carretOffset);

    void run();
    void execInstruction(Instruction instruction);
    void execInstruction(std::size_t instructionNumber);

    void printAllInstructions() const;
    void printAllRegisters() const;

    inline void initRegister(RegNumber number, RegValue value);
    inline void setRegisterValue(RegNumber number, RegValue value);
    inline RegValue registerValue(RegNumber number);

private:
    std::vector<Instruction> *mInstructions;
    std::map<RegNumber, RegValue> *mRegisters;
};

//-- interpreter exceptions
class AbsentInstructionException: public std::exception{};
class JumpToInstructionException: public std::exception
{
public:
    JumpToInstructionException(std::size_t number):
        instructionNumber(number){}

    std::size_t instructionNumber;
};


#endif // INTERPRETER_H
