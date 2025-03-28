#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <vector>
namespace fs = std::filesystem;
std::string SearchExecutable(const std::string &executable_name, const std::string &env_p)
{
    std::stringstream ss(env_p);
    std::vector<std::string> paths;
    std::string p;
    while (std::getline(ss, p, ':'))
    {
        paths.push_back(p);
    }
    for (const auto &path : paths)
    {
        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(path))
            {
                if (entry.is_regular_file() &&
                    entry.path().filename() == executable_name)
                {
                    auto perms = entry.status().permissions();
                    if ((perms & fs::perms::owner_exec) != fs::perms::none ||
                        (perms & fs::perms::group_exec) != fs::perms::none ||
                        (perms & fs::perms::others_exec) != fs::perms::none)
                    {
                        return entry.path().c_str();
                    }
                }
            }
        }
        catch (const std::exception &ex)
        {
        }
    }
    return "";
}
std::string EchoMessage(const std::string &params)
{
    std::string result = "";
    if ((params.at(0) == '\'') && (params.at(params.size() - 1) == '\''))
    {
        result = params.substr(1, params.size() - 1);
        result = result.substr(0, result.size() - 1);
    }
    else
    {
        bool space_found = false;
        bool apos_start = false;
        for (auto c : params)
        {
            if (c == ' ' && !apos_start) // For any spaces not enclosed by apostrophes
            {
                if (!space_found)
                    space_found = true;
                else
                    continue; // More spaces -> ignore them
            }
            else if (space_found)
                space_found = false; // No more spaces to handle
            if (c == '\"')
            {
                apos_start = !apos_start;
                continue;
            }
            result += c;
        }
    }
    return result;
}
int main()
{
    std::string env_p = std::string(getenv("PATH"));
    while (true)
    {
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;
        std::string input;
        std::cout << "$ ";
        std::getline(std::cin, input);
        std::string exec_name = input.substr(0, input.find(' '));
        std::string params = input.substr(input.find(' ') + 1,
                                          input.size() - input.find(' ') + 1);
        if (input == "exit 0")
            return 0;
        if (exec_name == "echo")
        {
            std::cout << EchoMessage(params) << std::endl;
            continue;
        }
        if (exec_name == "type" && ((params == "echo") || (params == "exit") ||
                                    (params == "type") || (params == "pwd")))
        {
            std::cout << params << " is a shell builtin" << std::endl;
            continue;
        }
        if (exec_name == "type")
        {
            std::string exec_path = SearchExecutable(params, env_p);
            if (exec_path == "")
                std::cout << params << ": not found" << std::endl;
            else
            {
                std::cout << params << " is " << exec_path << std::endl;
            }
            continue;
        }
        if (exec_name == "cd")
        {
            if (params == "~")
            {
                chdir(getenv("HOME"));
                continue;
            }
            if (fs::exists(fs::path(params)))
            {
                chdir(params.c_str());
                continue;
            }
            std::cout << exec_name << ": " << params
                      << ": No such file or directory" << std::endl;
            continue;
        }
        if (SearchExecutable(exec_name, env_p) != "")
        {
            system(input.c_str());
            continue;
        }
        if (input == "pwd")
        {
            system(input.c_str());
            continue;
        }
        std::cout << exec_name << ": not found" << std::endl;
    }
}