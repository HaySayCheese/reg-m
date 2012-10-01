#include "interpreter.h"

Instruction::Instruction(Instruction::Type type, RegNumber reg1, RegNumber reg2, InstructionPos instr):
    mType(type)
{
    switch (type){
    case Instruction::CT_Z:
    case Instruction::CT_S:
        this->arg1  = reg1;
        break;

    case Instruction::CT_T:
        this->arg1  = reg1;
        this->arg2  = reg2;
        break;

    case Instruction::CT_J:
        this->arg1  = reg1;
        this->arg2  = reg2;
        this->instr = instr;
        break;

#ifdef DEBUG
    default:
        std::cout << "ERROR: (Command::Command) - invalid command type;";
#endif
    }
}

void Instruction::setType(Instruction::Type type)
{
    mType = type;
}

Instruction::Type Instruction::type() const
{
    return mType;
}


Interpreter::Interpreter()
{
    // Creating containers for instructions and registers.
    // Because this containers may be as big as possible thay are created on heap.

    mInstructions = new std::vector<Instruction>();
    mRegisters = new std::map<RegNumber, RegValue>();
}

bool Interpreter::parseFile(std::string fileName)
{
    if (fileName.empty()){
        std::cout << "No input file specified. Process stopped." << std::endl;
        return false;
    }

    std::ifstream inputFile(fileName.c_str());
    if (! inputFile){
        std::cout << "Can't open file \"" << fileName << "\". Process stopped." << std::endl;
        return false;
    }

    // parse file and execute instructions
    std::string line;
    std::size_t lineNumber = 0;
    bool result = true;

    while (std::getline(inputFile, line)){
        try {
            ++lineNumber;
            parseLine(line);

        } catch(EmptyCommandExpcept &){
            // ignore empty line
            continue;

        } catch (InvalidCommandSyntaxExcept &e){
            std::cout << "Parse error at " << "[" << lineNumber << "; " << e.index() << "]: "
                      << e.what() << std::endl;
            result = false;
            continue;

        } catch(std::exception &){
            std::cout << "Parse error at " << "[" << lineNumber << "; ?]: Unknown error." << std::endl;
            result = false;
            continue;
        }
    }
    inputFile.close();

    if (! result){
        std::cout << "Process stopped. File \"" << fileName.c_str()
                  << "\" contains invalid instructions and can't be executed." << std::endl;
        return false;
    }

    // if some of registers was inititalised before instructions - print their values.
    if (mRegisters->size() > 0){
        std::cout << "Register's initial values: " << std::endl;
        printAllRegisters();
    }

    // if instructions count > 0 - print all instructions
    // else - stop the interpreter.
    if (mInstructions->size() > 0){
        std::cout << std::endl << "Instructions: " << std::endl;
        printAllInstructions();
    } else {
        std::cout << std::endl << "No instructions occured. Process stoped." << std::endl;
        return false;
    }

    run();
    return true;
}

void Interpreter::execInstruction(Instruction instruction)
{
    switch (instruction.type()) {
    case Instruction::CT_Z:
        // set register to zero.
        setRegisterValue(instruction.arg1, 0);
        break;

    case Instruction::CT_S:
        // increment register.
        setRegisterValue(instruction.arg1, registerValue(instruction.arg1)+1);
        break;

    case Instruction::CT_T:
        // move R[n]=R[m].
        setRegisterValue(instruction.arg2, registerValue(instruction.arg1));
        break;

    case Instruction::CT_J:
        // jump to specified command if reg1 == reg2,
        // else - execute next command in order.

        RegValue firstRegValue = registerValue(instruction.arg1);
        RegValue secondRegValue = registerValue(instruction.arg2);

        if (firstRegValue == secondRegValue)
            throw JumpToInstructionException(instruction.instr-1);
        break;
    }
}

bool Interpreter::parseLine(std::string instruction)
{
    if (instruction.empty())
        throw EmptyCommandExpcept();

    std::size_t pos = 0;

    // ignoring start space symbols
    for (; pos<instruction.length(); ++pos){
        if (instruction.at(pos) != ' ')
            break;
    }

    static bool isInitialisation = true;
    // check for initial instruction
    if (instruction[pos] == 'r' || instruction[pos] == 'R'){
        if (! isInitialisation)
            throw InvalidCommandSyntaxExcept("Initialisation instructions not allowed here.", pos+1);

        ++pos;
        return parseInitInstruction(instruction.substr(pos),pos+1);
    }

    // check for regular instruction
    else if (instruction[pos] == 'z' || instruction[pos] == 'Z'){
        isInitialisation = false;
        ++pos;
        return parseInstruction(instruction.substr(pos), Instruction::CT_Z, pos+1);

    } else if (instruction[pos] == 's' || instruction[pos] == 'S'){
        isInitialisation = false;
        ++pos;
        return parseInstruction(instruction.substr(pos), Instruction::CT_S, pos+1);

    } else if (instruction[pos] == 't' || instruction[pos] == 'T'){
        isInitialisation = false;
        ++pos;
        return parseInstruction(instruction.substr(pos), Instruction::CT_T, pos+1);

    } else if (instruction[pos] == 'j' || instruction[pos] == 'J'){
        isInitialisation = false;
        ++pos;
        return parseInstruction(instruction.substr(pos), Instruction::CT_J, pos+1);
    }

    throw InvalidCommandSyntaxExcept("Invalid symbol occurred.", pos+1);
}

bool Interpreter::parseInstruction(std::string instruction, Instruction::Type commandType, std::size_t carretOffset)
{
    std::size_t pos = 0;


    // ignoring space symbols betwen instruction and open brace
    // and check if last processed symbol was not last in the instruction
    for (; pos<instruction.length(); ++pos){
        if (instruction.at(pos) != ' ')
            break;
    }
    if (instruction.length() == pos)
        throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

    // check for open parenthesis
    if (instruction.at(pos) != '(')
        throw InvalidCommandSyntaxExcept("Invalid syntax. Open parenthesis is expected.", pos+carretOffset);
    ++pos;

    Instruction command(commandType);


    //-- parsing first argument
    // ignoring space symbols betwen instruction and open brace
    // and check if last processed symbol was not last in the instruction
    for (; pos<instruction.length(); ++pos){
        if (instruction.at(pos) != ' ')
            break;
    }
    if (instruction.length() == pos)
        throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

    // read argument
    std::string arg1;
    for (; pos<instruction.length(); ++pos){
        if ('0' == instruction.at(pos) ||
                '1' == instruction.at(pos) ||
                '2' == instruction.at(pos) ||
                '3' == instruction.at(pos) ||
                '4' == instruction.at(pos) ||
                '5' == instruction.at(pos) ||
                '6' == instruction.at(pos) ||
                '7' == instruction.at(pos) ||
                '8' == instruction.at(pos) ||
                '9' == instruction.at(pos) ){

            arg1.push_back(instruction.at(pos));
            continue;

        } else {
            // ignoring space symbols between first argument and comma
            // and check if last processed symbol was not last in the instruction
            for (; pos<instruction.length(); ++pos){
                if (instruction.at(pos) != ' ')
                    break;
            }
            if (instruction.length() == pos)
                throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

            if (command.type() == Instruction::CT_Z || command.type() == Instruction::CT_S){
                if (instruction.at(pos) != ')')
                    throw InvalidCommandSyntaxExcept("Invalid symbol occurred. Argument or close parenthesis expected.", pos+carretOffset);
                else {
                    if (arg1.empty())
                        throw InvalidCommandSyntaxExcept("First argument can't be empty.", pos+carretOffset);

                    command.arg1 = atoll(arg1.c_str());
                    mInstructions->push_back(command);
                    return true;
                }
            }

            if (instruction.at(pos) != ',')
                throw InvalidCommandSyntaxExcept("Invalid symbol occurred. Argument or comma expected.", pos+carretOffset);
            else {
                // check if first argument is not empty
                if (arg1.empty())
                    throw InvalidCommandSyntaxExcept("First argument can't be empty.", pos+carretOffset);

                command.arg1 = atoll(arg1.c_str());
                if (command.type() == Instruction::CT_Z || command.type() == Instruction::CT_S){
                    mInstructions->push_back(command);
                    return true;
                }

                ++pos;
                break;
            }
        }
    }

    //- second argument
    // ignoring space symbols between comma and and second argument
    for (; pos<instruction.length(); ++pos){
        if (instruction.at(pos) != ' ')
            break;
    }
    // check if last processed symbol was not last in the instruction
    if (instruction.length() == pos)
        throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

    // read argument
    std::string arg2;
    for (; pos<instruction.length(); ++pos){
        if ('0' == instruction.at(pos) ||
                '1' == instruction.at(pos) ||
                '2' == instruction.at(pos) ||
                '3' == instruction.at(pos) ||
                '4' == instruction.at(pos) ||
                '5' == instruction.at(pos) ||
                '6' == instruction.at(pos) ||
                '7' == instruction.at(pos) ||
                '8' == instruction.at(pos) ||
                '9' == instruction.at(pos) ){

            arg2.push_back(instruction.at(pos));
            continue;

        } else {
            // ignoring space symbols between second argument and comma
            // and check if last processed symbol was not last in the instruction
            for (; pos<instruction.length(); ++pos){
                if (instruction.at(pos) != ' ')
                    break;
            }
            if (instruction.length() == pos)
                throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

            if (command.type() == Instruction::CT_T){
                if (instruction.at(pos) != ')')
                    throw InvalidCommandSyntaxExcept("Invalid symbol occurred. Argument or close parenthesis expected.", pos+carretOffset);
                else {
                    if (arg2.empty())
                        throw InvalidCommandSyntaxExcept("Second argument can't be empty.", pos+carretOffset);

                    command.arg2 = atoll(arg2.c_str());
                    mInstructions->push_back(command);
                    return true;
                }
            }

            if (instruction.at(pos) != ',')
                throw InvalidCommandSyntaxExcept("Invalid symbol occurred. Argument or comma expected.", pos+carretOffset);
            else {
                // check if second argument is not empty
                if (arg2.empty())
                    throw InvalidCommandSyntaxExcept("Second argument can't be empty.", pos+carretOffset);

                command.arg2 = atoll(arg2.c_str());
                ++pos;
                break;
            }
        }
    }

    //- third argument
    // ignoring space symbols between comma and and third argument
    // and check if last processed symbol was not last in the instruction
    for (; pos<instruction.length(); ++pos){
        if (instruction.at(pos) != ' ')
            break;
    }
    if (instruction.length() == pos)
        throw InvalidCommandSyntaxExcept("Unexpected end of instruction occurred.", pos+carretOffset);

    // read argument
    std::string arg3;
    for (; pos<instruction.length(); ++pos){
        if ('0' == instruction.at(pos) ||
                '1' == instruction.at(pos) ||
                '2' == instruction.at(pos) ||
                '3' == instruction.at(pos) ||
                '4' == instruction.at(pos) ||
                '5' == instruction.at(pos) ||
                '6' == instruction.at(pos) ||
                '7' == instruction.at(pos) ||
                '8' == instruction.at(pos) ||
                '9' == instruction.at(pos) ){

            arg3.push_back(instruction.at(pos));
            continue;

        } else {
            if (arg3.empty())
                throw InvalidCommandSyntaxExcept("Third argument can't be empty.", pos+carretOffset);

            command.instr = atoll(arg3.c_str());

            // ignoring space symbols between third argument and close parenthesis
            for (; pos<instruction.length(); ++pos){
                if (instruction.at(pos) != ' ')
                    break;
            }
            if (instruction.length() == pos || instruction.at(pos) != ')')
                throw InvalidCommandSyntaxExcept("Invalid symbol occurred. Close parenthesis expected.", pos+carretOffset);

            mInstructions->push_back(command);
            return true;
        }
    }

    return false;
}

bool Interpreter::parseInitInstruction(std::string instruction, std::size_t carretOffset)
{
    std::size_t pos = 0;

    // parse register's number
    for (; pos<instruction.length(); ++pos){
        if (' ' != instruction.at(pos))
            break;
    }
    if (pos == instruction.length())
        throw InvalidCommandSyntaxExcept("Unexpected end of initialising instruction.", pos+carretOffset);

    std::string regNumber;
    for (; pos<instruction.length(); ++pos){
        if ('0' == instruction.at(pos) ||
                '1' == instruction.at(pos) ||
                '2' == instruction.at(pos) ||
                '3' == instruction.at(pos) ||
                '4' == instruction.at(pos) ||
                '5' == instruction.at(pos) ||
                '6' == instruction.at(pos) ||
                '7' == instruction.at(pos) ||
                '8' == instruction.at(pos) ||
                '9' == instruction.at(pos) ){

            regNumber.push_back(instruction.at(pos));
            continue;
        } else {
            for (; pos<instruction.length(); ++pos){
                if (' ' != instruction.at(pos))
                    break;
            }
            if (pos == instruction.length())
                throw InvalidCommandSyntaxExcept("Unexpected end of initialising instruction.", pos+carretOffset);

            if ('=' != instruction.at(pos))
                 throw InvalidCommandSyntaxExcept("Invalid symbol occurred.", pos+carretOffset);

            if (regNumber.empty())
                throw InvalidCommandSyntaxExcept("Register's number can't be empty.", pos+carretOffset);

            ++pos;
            break;
        }
    }

    // parse register's initial value
    for (; pos<instruction.length(); ++pos){
        if (' ' != instruction.at(pos))
            break;
    }
    if (pos == instruction.length())
        throw InvalidCommandSyntaxExcept("Unexpected end of initialising instruction.", pos+carretOffset);

    std::string regValue;
    for (; pos<instruction.length(); ++pos){
        if ('0' == instruction.at(pos) ||
                '1' == instruction.at(pos) ||
                '2' == instruction.at(pos) ||
                '3' == instruction.at(pos) ||
                '4' == instruction.at(pos) ||
                '5' == instruction.at(pos) ||
                '6' == instruction.at(pos) ||
                '7' == instruction.at(pos) ||
                '8' == instruction.at(pos) ||
                '9' == instruction.at(pos) ){

            regValue.push_back(instruction.at(pos));
            continue;
        }
        break;
    }
    if (regValue.empty())
        throw InvalidCommandSyntaxExcept("Register's value can't be empty.", pos+carretOffset);

    // initialise the register
    setRegisterValue(atoll(regNumber.c_str()), atoll(regValue.c_str()));
    return true;
}

void Interpreter::printAllInstructions() const
{
    std::vector<Instruction>::const_iterator it = mInstructions->begin();
    std::size_t number=0;

    for (; it != mInstructions->end(); ++it, ++number){
        std::cout << "[ins " << number+1 << "]: ";
        switch ((*it).type()) {
        case Instruction::CT_Z:
            std::cout << "Z(" << (*it).arg1 << ")" << std::endl;
            break;

        case Instruction::CT_S:
            std::cout << "S(" << (*it).arg1 << ")" << std::endl;
            break;

        case Instruction::CT_T:
            std::cout << "T(" << (*it).arg1 << ", " << (*it).arg2 << ")" << std::endl;
            break;

        case Instruction::CT_J:
            std::cout << "J(" << (*it).arg1 << ", " << (*it).arg2 << ", " << (*it).instr << ")" << std::endl;
            break;
        }
    }
}

void Interpreter::printAllRegisters() const
{
    std::map<RegNumber, RegValue>::const_iterator it = mRegisters->begin();
    for (; it != mRegisters->end(); ++it){
        std::cout << "[reg " << (*it).first << "]: " << (*it).second << std::endl;
    }
}

void Interpreter::run()
{
    std::size_t nextCommand = 0;

    while (nextCommand < mInstructions->size()){
        try {
            Instruction instruction = mInstructions->at(nextCommand);

            try {
                execInstruction(instruction);
                ++nextCommand;

            } catch (JumpToInstructionException &e){
                nextCommand = e.instructionNumber;
                continue;
            }

        } catch (std::bad_alloc &) {
            std::cout << "ERROR: Not enough system memory. Process stopped.";
            return;
        } catch(std::exception &) {
            std::cout << "Unknown error occured. Process stopped.";
            return;
        }
    }

    std::cout << std::endl << "Program terminated on instruction " << nextCommand+1 << " with results: " << std::endl;
    printAllRegisters();
}

void Interpreter::setRegisterValue(RegNumber number, RegValue value)
{
    if (mRegisters->find(number) == mRegisters->end())
        mRegisters->insert(std::pair<RegNumber, RegValue>(number, value));
    else
        mRegisters->at(number) = value;
}

RegValue Interpreter::registerValue(RegNumber number)
{
    if (mRegisters->find(number) == mRegisters->end()){
        mRegisters->insert(std::pair<RegNumber, RegValue>(number, 0));
        return 0;
    }
    else
        return mRegisters->at(number);
}
