#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
std::unordered_map<std::string, bool> builtInCommands;
void initCommands()
{
  builtInCommands["echo"] = true;
  builtInCommands["type"] = true;
  builtInCommands["exit"] = true;
  builtInCommands["pwd"] = true;
  builtInCommands["cat"] = true;
}
std::string getPath(std::string command)
{
  std::string newCommand;
  std::string chFront, chBack;
  if(command.front() == '\'' || command.front() == '\"') {
    if(command.size() > 0 && (command.front() == '\'' || command.front() == '\"'))
      newCommand = command.substr(1);
    if(command.size() > 0 && (command.back() == '\'' || command.back() == '\"'))
      newCommand = newCommand.substr(0, newCommand.size()-1);
    chFront = command.front();
    chBack = command.back();
  }
  else {
    newCommand = command;
    chFront = "";
    chBack = "";
  }
  std::string pathEnv = std::getenv("PATH");
  std::stringstream pathStream(pathEnv);
  std::string path;
  while (!pathStream.eof())
  {
    std::getline(pathStream, path, ':');
    std::string fullPath = path + "/" + newCommand;
    if (std::filesystem::exists(fullPath))
    {
      return path + "/" + chFront + newCommand + chBack;
    }
  }
  return "";
}
void handleNotFound(std::string command)
{
  std::cerr << command << ": command not found" << std::endl;
}
void handleEcho(std::string argument)
{
  char check = '\0';
  std::vector<std::string> printable;
  std::string word = "";
  bool isInsideQuotes = false;
  for(int i = 0; i < argument.size(); i++) {
    if(check == 'd' && argument[i] == '\"')
    {
      if(check == 'd')
        check = '\0';
      if(argument[i] == '\'')
        isInsideQuotes = !isInsideQuotes;
      if(i+1 < argument.size() && argument[i+1] == ' ')
        word += ' ';
      if(i > 0)
        printable.push_back(word);
      word = "";
    }
    else if (check == 'c' && argument[i] == '\'')
    {
      if(check == 'c')
        check = '\0';
      if(i+1 < argument.size() && argument[i+1] == ' ')
        word += ' ';
      if(i > 0)
        printable.push_back(word);
      word = "";
    }
    else if(check == '\0') {
      if(argument[i] == ' ' && word.size() > 0) {
        word += ' ';
        printable.push_back(word);
        word = "";
      }
      else if(argument[i] == '\"') {
        check = 'd';
      }
      else if(argument[i] == '\'') {
        check = 'c';
      }
      else if(argument[i] != ' ') {
        if(argument[i] == '\\') {
          word += argument[i+1];
          i++;
        }
        else
          word += argument[i];
      }
    }
    else if (check == 'c' || check == 'd')
    {
      if(check == 'd' && argument[i] == '\\' && i+1 < argument.size() && (argument[i+1] == '\'' || argument[i+1] == '\"' || argument[i+1] == '\\')) {
        word += argument[i+1];
        i++;
      }
      else
        word += argument[i];
    }
  }
  if(word.size() > 0)
    printable.push_back(word);
  std::string toPrint = "";
  for(auto iword : printable)
  {
    toPrint += iword;
  }
  std::cout << toPrint << std::endl;
}
void handleType(std::string argument)
{
  if (builtInCommands[argument]) {
    if(argument == "cat")
      std::cout << "cat is /usr/bin/cat" << std::endl;
    else
      std::cout << argument << " is a shell builtin" << std::endl;
  }
  else
  {
    std::string path = getPath(argument);
    if (path.empty())
      std::cerr << argument << ": not found" << std::endl;
    else
      std::cout << argument << " is " << path << std::endl;
  }
}
void handleExecutablePath(std::string command, std::string argument)
{
  std::string path = getPath(command);
  if (path.empty())
    handleNotFound(command);
  else
  {
    std::string execCommand = "exec " + path + " " + argument;
    std::system(execCommand.c_str());
  }
}
void handlePwd()
{
  std::string currentPath = std::filesystem::current_path().string();
  std::cout << currentPath << std::endl;
}
void handleCd(std::string argument)
{
  if (std::filesystem::exists(argument))
  {
    std::filesystem::current_path(argument);
  }
  else if (argument == "~")
  {
    std::filesystem::current_path(std::getenv("HOME"));
  }
  else
  {
    std::cerr << "cd: " << argument << ": No such file or directory" << std::endl;
  }
}
void handleCat(std::string argument)
{
  std::vector<std::string> files;
  char check = '\0';
  std::string file_path = "";
  for (int i = 0; i < argument.size(); i++)
  {
    if(check == 'd' && argument[i] == '\"')
    {
      if(check == 'd')
        check = '\0';
      if (i > 0)
        files.push_back(file_path);
      file_path = "";
    }
    else if (check == 'c' && argument[i] == '\'')
    {
      if(check == 'c')
        check = '\0';
      if (i > 0)
        files.push_back(file_path);
      file_path = "";
    }
    else if(check == '\0') {
      if(argument[i] == ' ' && file_path.size() > 0) {
        files.push_back(file_path);
        file_path = "";
      }
      else if(argument[i] == '\"') {
        check = 'd';
      }
      else if(argument[i] == '\'') {
        check = 'c';
      }
      else if(argument[i] != ' ') {
        file_path += argument[i];
      }
    }
    else if (check == 'c' || check == 'd')
    {
      // if(check == 'd' && argument[i] == '\\' && i+1 < argument.size() && (argument[i+1] == '\"' || argument[i+1] == '\'' || argument[i+1] == '\\' || argument[i+1] == ' ')) {
      //   file_path += argument[i+1];
      //   i++;
      // }
      // else
        file_path += argument[i];
    }
  }
  if(file_path.size() > 0)
    files.push_back(file_path);
  for (int i = 0; i < files.size(); i++)
  {
    std::string ifile = files[i];
    std::ifstream file(ifile);
    if (file.is_open())
    {
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string fileContent = buffer.str();
      std::cout << fileContent;
      file.close();
      }
    else
    {
      std::cerr << "cat: " << ifile << ": No such file or directory" << std::endl;
    }
  }
}
bool handleOutput(std::string input)
{
  int i = 1;
  if(input.front() == '\'' || input.front() == '\"')
  {
    char ch = input.front();
    while(i < input.size() && input[i] != ch)
      i++;
  }
  int separator = input.find(" ", i);
  if (separator == -1)
  {
    separator = input.size();
  }
  std::string command = separator == input.size() ? input : input.substr(0, separator);
  std::string argument = separator < input.size() - 1 ? input.substr(separator + 1) : "";
  if (command == "exit")
  {
    return false;
  }
  if (command == "echo")
  {
    handleEcho(argument);
    return true;
  }
  else if (command == "type")
  {
    handleType(argument);
    return true;
  }
  else if (command == "pwd")
  {
    handlePwd();
    return true;
  }
  else if (command == "cd")
  {
    handleCd(argument);
    return true;
  }
  else if (command == "cat")
  {
    handleCat(argument);
    return true;
  }
  else
  {
    handleExecutablePath(command, argument);
    return true;
  }
  return false;
}
int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  initCommands();
  while (true)
  {
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);
    bool shouldContinue = handleOutput(input);
    if (shouldContinue)
    {
      continue;
    }
    else
    {
      return 0;
    }
  }
}