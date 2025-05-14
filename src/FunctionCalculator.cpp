#include "FunctionCalculator.h"
#include "SquareMatrix.h"
#include "Add.h"
#include "Sub.h"
#include "Comp.h"
#include "Identity.h"
#include "Transpose.h"
#include "Scalar.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

FunctionCalculator::FunctionCalculator(std::istream& istr, std::ostream& ostr)
    : m_actions(createActions()), m_operations(createOperations()), m_istr(istr), m_ostr(ostr)
{
}


void FunctionCalculator::run()
{
        do
        {
            try {
                if (!m_isMaxFunc) {
                    getOperationSize();
                }
                else{
                    m_ostr << '\n';
                    printOperations();
                    m_ostr << "Enter command ('help' for the list of available commands): ";
                    const auto action = readAction(m_istr);
                    runAction(action, m_istr);
                }
            }
            catch (const std::exception& e)
            {
                m_ostr << "Error: " << e.what() << '\n';
            }
            catch (...)
            {
                m_ostr << "Unknown error occurred\n";
            }
        } while (m_running);
}


void FunctionCalculator::eval(std::istream& in)
{
    try {
        if (auto index = readOperationIndex(in); index)
        {
            const auto& operation = m_operations[*index];
            int inputCount = operation->inputCount();
            int size = 0;
            in >> size;
			if (size <= 0 || size > 5)
			{
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Invalid input: plase enter size between 1 - 5");
			}
            if (in.peek() != '\n') {
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::invalid_argument("to meny argument for the action");
            }
            
            auto matrixVec = std::vector<Operation::T>();
            if (inputCount > 1)
                m_ostr << "\nPlease enter " << inputCount << " matrices:\n";

            for (int i = 0; i < inputCount; ++i)
            {
                auto input = Operation::T(size);
                m_ostr << "\nEnter a " << size << "x" << size << " matrix:\n";
                
                in >> input;
				if (in.peek() != '\n') {
                    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					throw std::invalid_argument("to many items for the matrix");
				}
                matrixVec.push_back(input);

            }
			auto result = operation->compute(matrixVec);
            m_ostr << "\n";
            operation->print(m_ostr, matrixVec);
			m_ostr << " = \n" << result;
        }
	}
	catch (const std::exception& e)
	{
		m_ostr << "Error: " << e.what() << '\n';
	}
    catch (...)
    {
        m_ostr << "Unknown error occurred\n";
    }
}


void FunctionCalculator::del(std::istream& in)
{
	
    if (auto i = readOperationIndex(in); i)
    {
        if (in.peek() != '\n' && in.peek() != 32) {
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw std::invalid_argument("to meny argument for the action");
        }
        m_operations.erase(m_operations.begin() + *i);
    }
}


void FunctionCalculator::help()
{
    m_ostr << "The available commands are:\n";
    for (const auto& action : m_actions)
    {
        m_ostr << "* " << action.command << action.description << '\n';
    }
    m_ostr << '\n';
}


void FunctionCalculator::exit()
{
    m_ostr << "Goodbye!\n";
    m_running = false;
}


void FunctionCalculator::printOperations() const
{
    m_ostr << "List of available matrix operations:\n";
    for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i)
    {
        m_ostr << i << ". ";
        m_operations[i]->print(m_ostr,true);
        m_ostr << '\n';
    }
    m_ostr << '\n';
    m_ostr << "Number of operations: " << m_operations.size() << "/" << m_operationSize << '\n' << '\n';
}


std::optional<int> FunctionCalculator::readOperationIndex(std::istream& in) const
{
    int i = 0;
	in >> i;
    if ((in.fail())) {
		in.clear();
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::invalid_argument("Invalid input: expected an integer for operation index");
    }

    if (i < 0 || i >= static_cast<int>(m_operations.size())) {
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::out_of_range("Operation index out of range");
    }

    return i;
}

FunctionCalculator::Action FunctionCalculator::readAction(std::istream& in) const {
    std::string actionStr;
    in >> actionStr;
    auto it = std::ranges::find(m_actions, actionStr, &ActionDetails::command);
    if (it != m_actions.end()) return it->action;
    throw std::invalid_argument("Unknown command: " + actionStr);
}

void FunctionCalculator::runAction(Action action, std::istream& in)
{
    switch (action)
    {
        default:
            m_ostr << "Unknown enum entry used!\n";
            break;

        case Action::Invalid:
            m_ostr << "Command not found\n";
            break;

        case Action::Eval:  
            eval(in);
            break;

        case Action::Add: 
			if (m_operations.size() >= m_operationSize)
			{
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Operation list is full");
			}
            binaryFunc<Add>(in);
            break;

        case Action::Sub:
            if (m_operations.size() >= m_operationSize) {
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Operation list is full");
            }
            binaryFunc<Sub>(in);
            break;

        case Action::Comp:    
			if (m_operations.size() >= m_operationSize) {
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Operation list is full");
			}
            binaryFunc<Comp>(in);
            break;

        case Action::Del:   
            del(in);
            break;

        case Action::Help:    
            help();            
            break;

        case Action::Exit:    
            exit();           
            break;

        case Action::Scal:     
			if (m_operations.size() >= m_operationSize) {
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Operation list is full");
			}
            unaryWithIntFunc<Scalar>(in);
            break;

        case Action::Read:       
            read();        
            break;

		case Action::Resize:
			setOperationSize(in);
			break;
    }
}


FunctionCalculator::ActionMap FunctionCalculator::createActions() const
{
    return ActionMap
    {
        {
            "eval",
            "(uate) num n - compute the result of function #num on an n×n matrix "
			"(that will be prompted)",
            Action::Eval
        },
        {
            "scal",
            "(ar) val - creates an operation that multiplies the "
			"given matrix by scalar val",
            Action::Scal
        },
        {
            "add",
            " num1 num2 - creates an operation that is the addition of the result of operation #num1 "
			"and the result of operation #num2",
            Action::Add
        },
         {
            "sub",
            " num1 num2 - creates an operation that is the subtraction of the result of operation #num1 "
			"and the result of operation #num2",
            Action::Sub
        },
        {
            "comp",
            "(osite) num1 num2 - creates an operation that is the composition of operation #num1 "
			"and operation #num2",
            Action::Comp
        },
        {
            "del",
            "(ete) num - delete operation #num from the operation list",
            Action::Del
        },
        {
            "help",
            " - print this command list",
            Action::Help
        },
        {
            "exit",
            " - exit the program",
            Action::Exit
        },
        {
            "read",
            " - read command from file",
			Action::Read
        },
        {
            "resize",
            " - resize the operation list",
            Action::Resize
        }
    };
}


FunctionCalculator::OperationList FunctionCalculator::createOperations() const
{
    return OperationList
    {
        std::make_shared<Identity>(),
        std::make_shared<Transpose>(),
    };
}

void FunctionCalculator::read() {
    std::string path;
    m_istr >> path;
    std::ifstream file(path);
    if (!file) throw std::invalid_argument("File not found");

    bool previousRunning = m_running;
    m_running = true;
    while (file && m_running) {
        try {
            if (file.peek() == std::ifstream::traits_type::eof()) {
                break;
            }
            Action action = readAction(file);
            runAction(action, file);
        }
        catch (const std::exception& e) {
            m_ostr << "Error in file: " << e.what() << '\n';
        }
    }
    m_running = previousRunning;
}

void FunctionCalculator::getOperationSize()
{
	m_ostr << "Please enter the size of the operation (2-100): ";
	m_istr >> m_operationSize;
	if (m_istr.fail()) {
		m_istr.clear();
		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		throw std::invalid_argument("Invalid input: expected an integer for operation size");
	}
	if (m_operationSize < 2 || m_operationSize > 100)
	{
		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		throw std::out_of_range("Invalid input: please enter size between 2 - 100");
	}
    m_isMaxFunc = true;
}

void FunctionCalculator::setOperationSize(std::istream& in)
{
    int newSize = 0;
    in >> newSize;
    if (newSize > m_operations.size()) {
        m_operationSize = newSize;
    }
    else {
        m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        m_ostr << "The new size is smaller than the current size. Do you want to delete the last operation? (y): ";
        char answer;
        m_istr >> answer;
        if (answer == 'y' || answer == 'Y') {
            for (int i = m_operationSize - 1; i >= newSize; --i) {
                m_operations.pop_back();
            }
            m_operationSize = newSize;
        }
        else {
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}
