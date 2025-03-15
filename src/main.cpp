#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage

  std::cout << "$ ";
  std::string input;

  while(std::getline(std::cin, input) && input.find("exit") != 0){
      if(input.find("echo ") == 0){
        const int ECHO_LEN = 5;
        std::string text = input.substr(ECHO_LEN);
        std::cout << text << std::endl;
      }else{
        std::cout << input << ": command not found" << std::endl;
      }
      std::cout<< "$ ";
    }
  exit(0);
}
