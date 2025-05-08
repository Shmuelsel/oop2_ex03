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


void FunctionCalculator::eval()
{
    try {
        if (auto index = readOperationIndex(); index)
        {
            const auto& operation = m_operations[*index];
            int inputCount = operation->inputCount();
            int size = 0;
            m_istr >> size;
			if (size <= 0 || size > 5)
			{
				m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Invalid input: plase enter size between 1 - 5");
			}
            //chack if there more input in the buffer
            if (m_istr.peek() != '\n') {
				m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::invalid_argument("to meny argument for the action");
            }
            
            auto matrixVec = std::vector<Operation::T>();
            if (inputCount > 1)
                m_ostr << "\nPlease enter " << inputCount << " matrices:\n";

            for (int i = 0; i < inputCount; ++i)
            {
                auto input = Operation::T(size);
                m_ostr << "\nEnter a " << size << "x" << size << " matrix:\n";
                m_istr >> input;
				//if there is more input then throw an exception
				if (m_istr.peek() != '\n') {
					m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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


void FunctionCalculator::del()
{
    if (auto i = readOperationIndex(); i)
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


std::optional<int> FunctionCalculator::readOperationIndex() const
{
    int i = 0;
	m_istr >> i;
    if ((m_istr.fail())) {
		m_istr.clear();
		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::invalid_argument("Invalid input: expected an integer for operation index");
    }

    if (i < 0 || i >= static_cast<int>(m_operations.size())) {
		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

        case Action::Eval:         eval();                     break;
        case Action::Add:          binaryFunc<Add>();          break;
        case Action::Sub:          binaryFunc<Sub>();          break;
        case Action::Comp:         binaryFunc<Comp>();         break;
        case Action::Del:          del();                      break;
        case Action::Help:         help();                     break;
        case Action::Exit:         exit();                     break;
        //case Action::Iden:          unaryFunc<Identity>();      break;
        //case Action::Tran:          unaryFunc<Transpose>();      break;
        case Action::Scal:          unaryWithIntFunc<Scalar>();      break;
			//need to add a read function from a file with path as a parameter
		//case Action::Read:         read();                     break;
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

//void FunctionCalculator::read()
//{
//	std::string path;
//	m_istr >> path;
//	if (path.empty())
//	{
//		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//		throw std::invalid_argument("Invalid input: expected a file path");
//	}
//	//read from file
//	std::ifstream file(path);
//	if (!file.is_open())
//	{
//		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//		throw std::invalid_argument("Invalid input: file not found");
//	}
//}