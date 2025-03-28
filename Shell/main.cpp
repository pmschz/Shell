#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <optional>
#include <numeric>
#include <fstream>
#include <termios.h>
#include <unistd.h>
std::unordered_set<std::string> commands = {
  "cd", "echo", "exit", "pwd", "type"
};
void split(const std::string& str, std::vector<std::string>& tokens, char delimiter) {
  std::stringstream ss(str);
  std::string token;
  while(std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
}
std::optional<std::string> findInPath(const std::vector<std::string>& pathDirectories, const std::string& command) {
  for(const auto& directory : pathDirectories) {
    std::filesystem::path path(directory);
    path /= command;
    if(std::filesystem::exists(path)) {
      return path.string();
    }
  }
  return std::nullopt;
}
std::optional<std::string> buildPath(const std::string& arg, std::string currentPath, const std::string& HOME_PATH) {
  std::vector<std::string> pathDirectories;
  split(arg, pathDirectories, '/');
  if(arg.front() == '/') {
    currentPath = "/";
  }
  for(const auto& directory : pathDirectories) {
    if(directory == "." || directory == "") {
      continue;
    }
    else if(directory == "~") {
      currentPath = std::filesystem::path(HOME_PATH).string();
    }
    else if(directory == "..") {
      currentPath = std::filesystem::path(currentPath).parent_path();
    }
    else {
      std::filesystem::path path(currentPath);
      path /= directory;
      if(std::filesystem::exists(path)) {
        currentPath = path.string();
      }
      else {
       return std::nullopt; 
      }
    }
    }
  return currentPath;
}
void handleSpecialCharsOutsideQuotes(std::string& s) {
  int posLastSeenQuote = -1;
  for(size_t i = 0; i + 1 < s.size(); ++i) {
    if(s[i] == '\"') {
      posLastSeenQuote *= -1;
    }
    else if(s[i] == '\\' && posLastSeenQuote == -1) {
      s.erase(i, 1);
    }
  }
}
std::string parseInputWithoutLiterals(const std::string& input) {
    std::istringstream iss(input);
    std::string token, parsedInput;
    while(iss >> token) {
      parsedInput += token + ' ';
    }
    parsedInput.pop_back();
    handleSpecialCharsOutsideQuotes(parsedInput);
    return parsedInput;
}
void handleSpecialChars(std::string& s) {
  for(size_t i = 0; i + 1 < s.size(); ++i) {
    if(s[i] == '\\' && (s[i + 1] == '\\' || s[i + 1] == '$' || s[i + 1] == '\"')) {
      s.erase(i, 1);
    }
  }
}
bool checkLiteral(const std::string& input, int pos, const char literal) {
  return input[pos] == literal &&
    (pos == 0 || input[pos - 1] != '\\' || (pos > 1 && input[pos - 1] == '\\' && input[pos - 2] == '\\'));
}
bool checkIfLiteralsExist(const std::string& args) {
  for(size_t i = 1; i < args.size(); ++i) {
    if(checkLiteral(args, i, '\"') || checkLiteral(args, i, '\'')) {
      return true;
    }
  }
  return false;
}
std::string parseInputWithLiterals(const std::string& input) {
  std::string newInput;
  size_t i = 0, j;
  while(i < input.size()) {
    if(checkLiteral(input, i, '\'')) {
      j = i + 1;
      while(j < input.size() && !checkLiteral(input, j, '\'')) {
        ++j;
      }
      if(j < input.size() + 1) {
        std::string literal = input.substr(i + 1, j - i - 1);
        newInput += literal;
      }
      i = j + 1;
      if(i < input.size() && input[i] != '\'') {
        newInput += ' ';
      }
    }
    else if(checkLiteral(input, i, '\"')) {
      j = i + 1;
      while(j < input.size() && !checkLiteral(input, j, '\"')) {
        ++j;
      }
      if(j < input.size() + 1) {
        std::string literal = input.substr(i + 1, j - i - 1);
        handleSpecialChars(literal);
        newInput += literal;
      }
      i = j + 1;
      if(i < input.size() && input[i] != '\"') {
        newInput += ' ';
      }
    }
    else {
      ++i;
    }
  }
  if(j < input.size()) {
    if(newInput.back() == ' ') newInput.pop_back();
    std::string strToAdd = input.substr(j + 1);
    handleSpecialChars(strToAdd);
    newInput += strToAdd;
  }
  return newInput;
}
void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void disableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void handleTabPress(std::string& input) {
  if(input == "ech") {
    input = "echo ";
    std::cout << "o ";
  }
  else if(input == "exi") {
    input = "exit ";
    std::cout << "t ";
  }
  else {
    //
  }
}
void readInputWithTabSupport(std::string& input) {
  enableRawMode();
  char c;
  while (true) {
    c = getchar();
    if (c == '\n') {
      std::cout << std::endl;
      break;
    } else if (c == '\t') {
      handleTabPress(input);
    } else if (c == 127) {
      if (!input.empty()) {
        input.pop_back();
        std::cout << "\b \b"; // Move cursor back, overwrite character with space, move cursor back again.
      }
    } else {
      input += c;
      std::cout << c;
    }
  }
  disableRawMode();
}
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::string currentPath = std::filesystem::current_path();
  const std::string HOME_PATH = std::getenv("HOME");
  std::vector<std::string> pathDirectories;
  split(std::getenv("PATH"), pathDirectories, ':');
  while(true) {
    std::cout << "$ ";
    std::string input;
    readInputWithTabSupport(input);
    // std::getline(std::cin, input);
    bool redirect = false, redirectError = false, append = false;
    std::string redirectPath = "";
    if(auto it = input.find('>'); it != std::string::npos) {
      if(it + 1 < input.size() && input[it + 1] == '>') {
        append = true;
        ++it;
      }
      redirectPath = input.substr(it + 2, input.size() - it - 2);
      if(append) --it;
      input = input.substr(0, it);
      if(input.back() == '1') {
        input.pop_back();
        redirect = true;
      }
      else if(input.back() == '2') {
        input.pop_back();
        redirectError = true;
      }
      else {
        redirect = true;
      }
      input.pop_back();
    }
    std::vector<std::string> tokens;
    split(input, tokens, ' ');
    if(tokens.empty()) continue;
    std::string args = std::accumulate(tokens.begin() + 1, tokens.end(), std::string(),
      [](const std::string& a, const std::string& b) {
        return a + (a.length() > 0 ? " " : "") + b;
      });
    std::string redirectionSpecifier = "";
    if(redirect) redirectionSpecifier += " 1>";
    else if(redirectError) redirectionSpecifier += " 2>";
    if(append) redirectionSpecifier += ">";
    redirectionSpecifier += " ";
    if(tokens.front() == "cat") {
      std::string command = tokens.front() + ' ' + args;
      command += redirectionSpecifier + redirectPath;
      std::system(command.c_str());
    }
    else if(tokens.front() == "cd") {
      auto newPath = buildPath(tokens[1], currentPath, HOME_PATH);
      if(newPath.has_value()) {
        currentPath = newPath.value();
      }
      else {
        std::cout << "cd: " << tokens[1] <<  ": No such file or directory" << std::endl;
      }
    }
    else if(tokens.front() == "echo") {
      std::string command;
      if(checkIfLiteralsExist(input)) {
        command = parseInputWithLiterals(input.substr(5));
      }
      else {
        command = parseInputWithoutLiterals(input).substr(5);
      }
      if(redirect) {
        std::filesystem::path newFilePath{redirectPath};
        std::ofstream file(newFilePath, (append ? std::ios_base::app : std::ios_base::out));
        if(file) {
          file << command << std::endl;
        }
      }
      else if(redirectError) {
        std::filesystem::path newFilePath{redirectPath};
        std::ofstream file(newFilePath, (append ? std::ios_base::app : std::ios_base::out));
        std::cout << command << std::endl;
      }
      else {
        std::cout << command << std::endl;
      }
    }
    else if(tokens.front() == "exit" && tokens[1] == "0") {
      return 0;
    }
    else if(tokens.front() == "ls") {
      std::string command = tokens.front() + ' ' + args;
      command += redirectionSpecifier + redirectPath;
      std::system(command.c_str());
    }
    else if(tokens.front() == "pwd") {
      std::cout << currentPath << std::endl;
    }
    else if(tokens.front() == "type") {
      std::string commandToType = tokens[1];
      if(commands.find(commandToType) != commands.end())
        std::cout << commandToType << " is a shell builtin" << std::endl;
      else if(auto path = findInPath(pathDirectories, commandToType); path.has_value())
        std::cout << path.value() << std::endl;
      else
        std::cout << commandToType << ": not found" << std::endl;
    }
    else if(input.front() == '\'') {
      size_t lastSingleQuote = input.find_last_of('\'');
        std::string filepath = input.substr(lastSingleQuote + 2);
        std::system(("cat " + filepath).c_str());
    }
    else if(input.front() == '\"') {
      size_t lastSingleQuote = input.find_last_of('\"');
        std::string filepath = input.substr(lastSingleQuote + 2);
        std::system(("cat " + filepath).c_str());
    }
    else if(auto commandPath = findInPath(pathDirectories, tokens.front()); commandPath.has_value()) {
      std::system((tokens.front() + ' ' + args).c_str());
    }
    else {
      std::cout << tokens.front() << ": command not found" << std::endl;
    }
  }
  return 0;
}