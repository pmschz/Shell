#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <filesystem>
#include <unistd.h>
namespace fs = std::filesystem;

using std::vector;
using std::string;

std::vector<std::string> gValid_commands = {"echo", "exit", "type", "cd"};
bool in_vector(std::vector<std::string> vec, std::string input){
  for (int i = 0; i < vec.size(); i++){
    if (vec[i] == input){
      return true;
    }
  }
  return false;
}
std::pair<string, string> split_first(string input){
  string first = "";
  string second = "";
  bool after_space = false;
  int size = input.size();
  for (int i = 0; i < size; i++){
    if (input[i] == ' ' && !after_space){
      after_space = true;
    } else if (!after_space) first += input[i];
    else second += input[i];
  }
  return {first, second};
}
vector<string> splitWith(string input, char delimiter){
  vector<string> result;
  string temp = "";
  for (int i = 0; i < input.size(); i++){
    if (input[i] == delimiter){
      result.push_back(temp);
      temp = "";
    } else temp += input[i];
  }
  result.push_back(temp);
  return result;
}
int commandStatus(string command, vector<string> paths){
  bool is_built_in = in_vector(gValid_commands, command);
  if (is_built_in) return 1;
  for (int i = 0; i < paths.size(); i++)
    if (fs::exists(paths[i] + "/" + command))
      return 2+i;
  return 0;
}
void echo(string input, vector<string> &paths){ std::cout << input << std::endl;}
void type(string input, vector<string> &paths){
  int status = commandStatus(input, paths);
  if (status == 1)
    std::cout << input << " is a shell builtin" << std::endl;
  else if (status >= 2)
    std::cout << input << " is " << paths[status-2]+"/"+input << std::endl;
  else std::cout << input << " not found" << std::endl;
}
void pwd(string input, vector<string> &paths){
  std::cout << fs::current_path() << std::endl;
}
void cd(string input, vector<string> &paths){
  if (input == "~")
    chdir(getenv("HOME"));
  else if (fs::exists(input))
    chdir(input.c_str());
  else std::cout << "cd: " << input << ": No such file or directory" << std::endl;
}
int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::pair<string, string> pairs;
  std::map<string, void(*)(string input, vector<string> &path)> builtins = {
    {"echo", echo},
    {"type", type},
    {"pwd", pwd},
    {"cd", cd}
  };
  const string path = getenv("PATH");
  //std::cout << path << std::endl;
  vector<string> paths = splitWith(path, ':');
  int status;
  while (true){
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);
    if (input == "exit 0")
      return 0;
    pairs = split_first(input);
    status = commandStatus(pairs.first, paths);
    if (status == 0){
      std::cout << input << ": command not found" << std::endl;
    }else {
      if (status == 1)
        builtins[pairs.first](pairs.second, paths);
      else {
        std::system((paths[status-2] + "/" + pairs.first + " " + pairs.second).c_str());
      }
    }
  }
}