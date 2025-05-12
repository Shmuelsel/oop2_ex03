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
            m_ostr << '\n';
            printOperations();
            m_ostr << "Enter command ('help' for the list of available commands): ";
            const auto action = readAction();
            runAction(action);
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
			//chack if there more input in the buffer only if "in" is from file and not from cin
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
				//if there is more input then throw an exception
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


FunctionCalculator::Action FunctionCalculator::readAction() const
{
    auto action = std::string();
    m_istr >> action;

    const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
    if (i != m_actions.end())
    {
        return i->action;
    }

    //return Action::Invalid;
	m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    throw std::invalid_argument("Unknown command: " + action);
}


void FunctionCalculator::runAction(Action action)
{
    switch (action)
    {
        default:
            m_ostr << "Unknown enum entry used!\n";
            break;

        case Action::Invalid:
            m_ostr << "Command not found\n";
            break;

        case Action::Eval:         eval(m_istr);                     break;
        case Action::Add:          binaryFunc<Add>(m_istr);          break;
        case Action::Sub:          binaryFunc<Sub>(m_istr);          break;
        case Action::Comp:         binaryFunc<Comp>(m_istr);         break;
        case Action::Del:          del(m_istr);                      break;
        case Action::Help:         help();                           break;
        case Action::Exit:         exit();                           break;
        case Action::Scal:         unaryWithIntFunc<Scalar>(m_istr); break;
        case Action::Read:         read();                           break;
    }
}


FunctionCalculator::ActionMap FunctionCalculator::createActions() const
{
    return ActionMap
    {
        {
            "eval",
            "(uate) num n - compute the result of function #num on an n׳n matrix "
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

void FunctionCalculator::read()
{
    std::string path;
    m_istr >> path;

    if (path.empty()) {
        m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::invalid_argument("Invalid input: expected a file path");
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::invalid_argument("Invalid input: file not found");
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        try {
            if (command == "eval") {
                std::string next_line;
                if (std::getline(file, next_line)) {
                    // שילוב השורה הנוכחית והשורה הבאה ב-iss
                    iss.clear(); // ניקוי מצב iss
                    iss.str(line + " " + next_line); // שילוב השורות
                    iss.seekg(0); // חזרה להתחלה של iss
                    iss >> command; // דילוג על המילה "eval" שוב, אם צריך
                    eval(iss);
                }
                else {
                    throw std::invalid_argument("Missing matrix elements for eval command");
                }
            }
            else if (command == "add") {
                binaryFunc<Add>(iss);
            }
            else if (command == "scal") {
				unaryWithIntFunc<Scalar>(iss);
            }
            else if (command == "sub") {
                binaryFunc<Sub>(iss);
            }
            else if (command == "comp") {
                binaryFunc<Comp>(iss);
            }
            else if (command == "del") {
                del(iss);
            }
            else if (command == "help") {
                help();
            }
            else if (command == "exit") {
                exit();
            }
            else {
                throw std::invalid_argument("Unknown command: " + command);
            }
        }
        catch (const std::exception& e) {
            m_ostr << "Error in line: " << line << "\nReason: " << e.what() << '\n';
        }
    }
}