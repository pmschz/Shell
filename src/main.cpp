#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage

  std::string input;

  while(input != "exit"){
    std::cout << "$ ";
    std::getline(std::cin, input);
    std::cerr << "nonexistent: command not found" << std::endl;
    std::cerr << input << ": command not found" << std::endl;
  }
  return 0;
}
