#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include "common/misc/CommandParser.h"
#include "common/misc/CommandInteractor.h"

using namespace zmc;

// 显示所有命令及其帮助信息
void showHelp(const CommandParser& parser) {
    std::cout << "Available commands:" << std::endl;
    
    // 获取所有命令的帮助文本
    auto commandHelp = parser.getAllCommandsHelp();
    auto commands = parser.getRegisteredCommands();
    
    // 找出最长的命令名称，用于对齐输出
    size_t maxCmdLength = 0;
    for (const auto& cmd : commands) {
        maxCmdLength = std::max(maxCmdLength, cmd.length());
    }
    
    // 按字母顺序排序命令
    std::sort(commands.begin(), commands.end());
    
    // 显示每个命令及其帮助文本
    for (const auto& cmd : commands) {
        std::cout << "  " << std::left << std::setw(maxCmdLength + 4) << cmd;
        auto it = commandHelp.find(cmd);
        if (it != commandHelp.end() && !it->second.empty()) {
            std::cout << "- " << it->second;
        }
        std::cout << std::endl;
    }
}

// 加法函数
void addNumbers(int a, int b) {
    std::cout << "Result: " << (a + b) << std::endl;
}

// 回显函数
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

// 乘法函数
float multiply(float a, float b) {
    return a * b;
}




    // 注册命令 - 使用统一的 bind 接口，添加帮助描述
void CommandInteractorBind(CommandParser& parser) {
    // 注册无参数函数
    parser.bind("help", [&parser]() { 
        showHelp(parser); 
    }, "Show this help message");
    
    // 注册退出命令
    parser.bind("exit", [&parser]() {
        std::cout << "Exiting..." << std::endl;
        parser.setExitFlag();
    }, "Exit the program");
    
    // 注册普通函数 (一个参数)
    parser.bind("echoSg", echoMessage, "Echo a message (usage: echo <message>)");
    parser.bind("echoPchar", echoPchar, "Echo a message (usage: echo <message>)");
    // 注册普通函数 (变长参数)
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
    // 注册普通函数 (两个参数)
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

    // 注册一个参数的 lambda
    parser.bind("upper", [](const std::string& text) {
        std::string result = text;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        std::cout << result << std::endl;
    }, "Convert text to uppercase (usage: upper <text>)");
    
    // 注册两个参数的 lambda
    parser.bind("divide", [](float a, float b) {
        if (b == 0) {
            std::cout << "Error: Division by zero" << std::endl;
        } else {
            std::cout << "Result: " << (a / b) << std::endl;
        }
    }, "Divide a by b (usage: divide <a> <b>)"); 
}


void CommandInteractor_demo() {
    // 创建命令解析器
    CommandParser parser;

    CommandInteractorBind(parser);

    // 创建命令交互器
    auto interactor = makeInteractor(parser);
    
    // 设置错误处理函数
    interactor.setErrorHandlerDefault();
    
    // 运行交互式命令循环
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