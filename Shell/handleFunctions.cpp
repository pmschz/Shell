#include <filesystem>
#include <iostream>
#include "handleFunctions.hpp"
#include "commands.hpp"
static std::string WORKING_DIR = "/app";
void handleEcho(const std::string& data) {
    std::cout << data << '\n';
}
void handleType(const std::string& command)
{
    if (get_command_enum(command) != executable)
        std::cout << command << " is a shell builtin\n";
    else {
        std::string exec_path = get_command_path(command);
        if (exec_path != "")
            std::cout << command << " is " << exec_path << '\n';
        else
            std::cout << command << ": not found\n";
    }
}
void handlePwd() {
    std::cout << WORKING_DIR << '\n';
}
void handleCd(const std::string& argument) {
    if (argument[0] == '/' && std::filesystem::exists(argument))
        WORKING_DIR = argument;
    else
        std::cout << argument << ": No such file or directory\n";
}
void handleExecutable(const std::string& command, const std::string& arguments)
{
    std::string exec_path = get_command_path(command);
    if (exec_path != "")
        system((command + " " + arguments).c_str());
    else
        std::cout << command << ": command not found\n";
}