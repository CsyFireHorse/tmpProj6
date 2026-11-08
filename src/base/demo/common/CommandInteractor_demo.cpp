#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include "common/misc/CommandParser.h"
#include "common/misc/CommandInteractor.h"

using namespace zmc;

// Display all commands and their help text.
void showHelp(const CommandParser& parser) {
    std::cout << "Available commands:" << std::endl;
    
    // Get the help text for all commands.
    auto commandHelp = parser.getAllCommandsHelp();
    auto commands = parser.getRegisteredCommands();
    
    // Find the longest command name to align the output.
    size_t maxCmdLength = 0;
    for (const auto& cmd : commands) {
        maxCmdLength = std::max(maxCmdLength, cmd.length());
    }
    
    // Sort commands alphabetically.
    std::sort(commands.begin(), commands.end());
    
    // Display each command and its help text.
    for (const auto& cmd : commands) {
        std::cout << "  " << std::left << std::setw(maxCmdLength + 4) << cmd;
        auto it = commandHelp.find(cmd);
        if (it != commandHelp.end() && !it->second.empty()) {
            std::cout << "- " << it->second;
        }
        std::cout << std::endl;
    }
}

// Addition function.
void addNumbers(int a, int b) {
    std::cout << "Result: " << (a + b) << std::endl;
}

// Echo function.
void echoMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void echoPchar(const char* message) {
    printf("echo %s\n", message);
}

static void echoVarArgs(const std::vector<std::string>& args) {
    std::string out;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) {
            out += ' ';
        }
        out += args[i];
    }
    std::cout << out << std::endl;
}

static void dumpArgv(int argc, char** argv) {
    // argv is a view; do not modify argv[i][j].
    std::cout << "argc=" << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "]=" << (argv[i] ? argv[i] : "(null)") << std::endl;
    }
}

// Multiplication function.
float multiply(float a, float b) {
    return a * b;
}




    // Register commands through the shared bind interface and add help descriptions.
void CommandInteractorBind(CommandParser& parser) {
    // Register a function with no arguments.
    parser.bind("help", [&parser]() { 
        showHelp(parser); 
    }, "Show this help message");
    
    // Register the exit command.
    parser.bind("exit", [&parser]() {
        std::cout << "Exiting..." << std::endl;
        parser.setExitFlag();
    }, "Exit the program");
    
    // Register a regular function with one argument.
    parser.bind("echoSg", echoMessage, "Echo a message (usage: echo <message>)");
    parser.bind("echoPchar", echoPchar, "Echo a message (usage: echo <message>)");
    // Register a regular function with variadic arguments.
    parser.bind("echoLambda", [](const std::vector<std::string>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            printf("echo %s\n", args[i].c_str());
        }
    }, "Echo a message using lambda (usage: echoLambda <message...>)");
    parser.bind("echo", echoVarArgs, "Echo a message (usage: echo <message...>)");
    // Demo binding for argc/argv style callback
    parser.bind("dump", dumpArgv, "Dump argc/argv (usage: dump <args...>)");
    
    parser.bind("inc", [](int a) {
        a++;
        std::cout << "Incremented to: " << a << std::endl; 
    });
    // Register a regular function with two arguments.
    parser.bind("add", addNumbers, "Add two integers (usage: add <a> <b>)");
    parser.bind("minus", [](double a, double b) {
        std::cout << "Result: " << (a - b) << std::endl;
    }, "Subtract b from a (usage: minus <a> <b>)");


    parser.bind("multiply", [](float a, float b) {
        float result = multiply(a, b);
        std::cout << "Result: " << result << std::endl;
    }, "Multiply two numbers (usage: multiply <a> <b>)");

    parser.bind("divide", [](float a, float b) {
        if (b == 0) {
            std::cout << "Error: Division by zero" << std::endl;
        } else {
            std::cout << "Result: " << (a / b) << std::endl;
        }
    }, "Divide a by b (usage: divide <a> <b>)");

    parser.bind("sum", [](const std::vector<std::string>& args) {
        int sum = 0;
        for (const auto& arg : args) {
            sum += std::stoi(arg);
        }
        std::cout << "Sum: " << sum << std::endl;
    }, "Sum the arguments (usage: sum <a> <b>)");

    // Register a lambda with one argument.
    parser.bind("upper", [](const std::string& text) {
        std::string result = text;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        std::cout << result << std::endl;
    }, "Convert text to uppercase (usage: upper <text>)");
    
    // Register a lambda with two arguments.
    parser.bind("divide", [](float a, float b) {
        if (b == 0) {
            std::cout << "Error: Division by zero" << std::endl;
        } else {
            std::cout << "Result: " << (a / b) << std::endl;
        }
    }, "Divide a by b (usage: divide <a> <b>)"); 
}


void CommandInteractor_demo() {
    // Create the command parser.
    CommandParser parser;

    CommandInteractorBind(parser);

    // Create the command interactor.
    auto interactor = makeInteractor(parser);
    
    // Set the error handler.
    interactor.setErrorHandlerDefault();
    
    // Run the interactive command loop.
    interactor.run("Simple Command Parser Demo\nType 'help' for available commands or 'exit' to quit");

} 

void CommandParse_demo(int argc, char** argv) {
    CommandParser parser;
    CommandInteractorBind(parser);
    auto result = parser.parseAndExecute(argc, argv);
    if (result != CommandParser::SUCCESS) {
        CommandParser::printError(result);
        return;
    }
}
