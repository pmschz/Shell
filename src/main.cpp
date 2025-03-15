#include <iostream>
#include <string>
int main() {
  bool exit = 0;
  while (!exit)
  {
    /* code */
  
  
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  //std::cout << "Logs from your program will appear here!\n";
  // Uncomment this block to pass the first stage
   std::cout << "$ ";
   bool checkValid = 0;
   std::string input;
   std::getline(std::cin, input);
   
   if(input == "exit 0"){
    checkValid = 1;
    exit = 1;
   }
   if(input.substr(0,5) == "type "){
      checkValid = 1;
      std::string cmd = input.substr(5);
      if(cmd.substr(0,4) == "type" || cmd.substr(0,4) == "exit" || cmd.substr(0,4) == "echo"){
        std::cout << cmd << " is a shell builtin\n";
      }else{
        std::cout << cmd << " not found\n";
      }
   }
   if(input.substr(0,5) == "echo "){
      checkValid = 1;
      std::string toPrint = input.substr(5);
      std::cout << toPrint << '\n';
   }
   if(checkValid == 0){
      std::cout << input << ": command not found\n";
   }
 }
 return 0;
}