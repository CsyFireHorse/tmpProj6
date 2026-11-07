#ifndef _ZMC_FB_CMD_SIMULATOR_H_
#define _ZMC_FB_CMD_SIMULATOR_H_

#include "AxFbCmdPlayer.h"
#include "common/misc/CommandParser.h"

namespace zmc {

/**
 * Command-line interactive simulator that extends AxFbCmdPlayer with interactive features
 */
class AxFbCmdSimulator : public AxFbCmdPlayer {
public:
    AxFbCmdSimulator();
    ~AxFbCmdSimulator();
    
    void printWelcomeMsg()const;
    // Interactive command interface
    void runCommandInteractor(const std::string& welcomeMsg = "Motion Command Simulator Interface");
    
    // Access the command parser
    CommandParser& getCommandParser() { return _cmdParser; }
    const CommandParser& getCommandParser() const { return _cmdParser; }

private:
    // Command parser instance
    CommandParser _cmdParser;
    
    // Initialize the command parser
    void initCommandParser();
    
    // Register command handlers
    void registerCommands();
};

} // namespace zmc

#endif // _ZMC_FB_CMD_SIMULATOR_H_
