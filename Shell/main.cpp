#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
std::string get_path(std::string command){
  std::string path_env = std::getenv("PATH");
  std::stringstream ss(path_env);
  std::string path;
  while(!ss.eof()){
    getline(ss, path, ':');
    std::string abs_path = path + "/" + command;
    if (std::filesystem::exists(abs_path)) {
        return abs_path;
    }
  }
  return "";
}
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  // Uncomment this block to pass the first stage
  while(1){
  std::cout << "$ ";
  
  std::string input;
  std::getline(std::cin, input);
  int end = input.find(" ");
  std::string command = input.substr(0,end);
  std::string arguments = input.substr(end+1);
  if (command.compare("exit")==0){
    return 0;
    break;
  }
  else if (command.compare("echo")==0){
    
      std::cout<<arguments<<std::endl;
  }
  else if (command.compare("type")==0){
      if(arguments.compare("echo") ==0 || arguments.compare("type") ==0 || arguments.compare("exit")==0)
        std::cout<<arguments<<" is a shell builtin"<<std::endl;
      else
        {
          std::string path = get_path(arguments);
          if(path.empty())
            std::cout<<arguments<<": not found\n";
          else
            std::cout<<arguments<<" is "<<path<<std::endl;
        }
  }
  else if (command.compare("pwd")==0){
    std::cout<<std::filesystem::current_path().string()<<std::endl;
  }
  else if (command.compare("cd")==0){
    std::string absolute_path;
    const std::string &path = arguments;
    if (path[0] == '/')
		{absolute_path = path;
    
	if (absolute_path.empty())
		return 0;
	if (chdir(absolute_path.c_str()) == -1)
		std::cout << "cd: " << path << ": No such file or directory" << std::endl;
    }
    else if (path[0]== '.'){
    std::string cwd = std::filesystem::current_path().string();
    std::string dir = cwd + '/' + path;
    cwd = std::filesystem::canonical(dir);
    if (chdir(cwd.c_str()) == -1)
		std::cout << "cd: " << path << ": No such file or directory" << std::endl;
    }
  }
  else{
    std::string path = get_path(command);
    if(path.empty())
            std::cout<< command <<": command not found"<<std::endl;
          else
            {
              std::string command = path + " " + arguments;
              int result = system(command.c_str());
              if (result == -1) {
              std::cerr << "Error executing the command" << std::endl;
            }
            }
    }
  }
}