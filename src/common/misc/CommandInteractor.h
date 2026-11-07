#ifndef COMMAND_INTERACTOR_H
#define COMMAND_INTERACTOR_H

#include <string>
#include <iostream>
#include <functional>

namespace zmc{
/**
 * @brief Command line interaction template class, encapsulates command interaction logic
 * 
 * @tparam ParserT Command parser type
 */
template<typename ParserT>
class CommandInteractor {
public:
    /**
     * @brief Constructor
     * 
     * @param parser Command parser
     * @param prompt Command prompt
     */
    CommandInteractor(ParserT& parser, const std::string& prompt = "> ")
        : parser_(parser), prompt_(prompt), errorHandler_(nullptr) {}

    /**
     * @brief Set error handler function
     * 
     * @param handler Error handler function
     */
    void setErrorHandler(std::function<void(typename ParserT::ErrorCode)> handler) {
        errorHandler_ = std::move(handler);
    }
    void setErrorHandlerDefault(){
        errorHandler_ = [](typename ParserT::ErrorCode errorCode){
            ParserT::printError(errorCode);
        };
    }
    /**
     * @brief Set command prompt
     * 
     * @param prompt Command prompt
     */
    void setPrompt(const std::string& prompt) {
        prompt_ = prompt;
    }

    /**
     * @brief Display welcome message
     * 
     * @param message Welcome message
     */
    void showWelcomeMessage(const std::string& message) {
        std::cout << message << std::endl;
    }

    /**
     * @brief Run single command
     * 
     * @param input Command input
     * @return true if should continue loop
     * @return false if should exit loop (only when parser marks exit)
     */
    bool runCommand(const std::string& input) {
        if (input.empty()) {
            return true;
        }
        
        auto result = parser_.parseAndExecute(input);
        if (result != ParserT::SUCCESS) {
            if (errorHandler_) {
                errorHandler_(result);
            }
            // Command failure doesn't cause loop exit, only exits when parser sets exit flag
        }
        
        // Only returns false when parser sets exit flag
        return !parser_.isExitFlagSet();
    }

    /**
     * @brief Run interactive command loop
     * 
     * @param welcomeMessage Optional welcome message
     */
    void run(const std::string& welcomeMessage = "") {
        if (!welcomeMessage.empty()) {
            showWelcomeMessage(welcomeMessage);
        }
        
        while (true) {
            std::cout << prompt_;
            std::string input;
            std::getline(std::cin, input);
            
            // Only exits loop when parser sets exit flag
            if (!runCommand(input)) {
                break;
            }
        }
    }

private:
    ParserT& parser_;
    std::string prompt_;
    std::function<void(typename ParserT::ErrorCode)> errorHandler_;
};

/**
 * @brief Helper function to create command line interactor
 * 
 * @tparam ParserT Command parser type
 * @param parser Command parser
 * @param prompt Command prompt
 * @return CommandInteractor<ParserT> Command line interactor
 */
template<typename ParserT>
CommandInteractor<ParserT> makeInteractor(ParserT& parser, const std::string& prompt = "> ") {
    return CommandInteractor<ParserT>(parser, prompt);
}


}
#endif // COMMAND_INTERACTOR_H 