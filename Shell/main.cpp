#include <iostream>
#include <set>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
using namespace std;

string get_path(string command, bool cQ = false, string Quote = "")
{
  char *path = getenv("PATH");
  string p = string(path);
  string pth = "";
  set<string> pathes;
  for (int i = 0; i < p.size(); i++)
  {
    if (p[i] == ':')
    {
      pathes.insert(pth);
      pth = "";
    }
    else
      pth += p[i];
  }
  pathes.insert(pth);
  for (string cmd : pathes)
  {
    string file = cmd + "/" + command;
    if (filesystem::exists(file))
    {
      string resolved_path = filesystem::canonical(file).string();
      if (cQ)
        return cmd + "/" + Quote + command + Quote;
      else
        return resolved_path;
    }
  }
  return "";
}
string get_basename(const string &path)
{
  return filesystem::path(path).filename().string();
}
bool is_exe(string command)
{
  string path = get_path(command);
  if (filesystem::exists(path))
  {
    auto perms = filesystem::status(path).permissions();
    return (perms & filesystem::perms::owner_exec) != filesystem::perms::none ||
           (perms & filesystem::perms::group_exec) != filesystem::perms::none ||
           (perms & filesystem::perms::others_exec) != filesystem::perms::none;
  }
  return false;
}
int containQuotes(string arg)
{
  // cout<<arg<<endl;
  // cout << arg[arg.size() - 1] << endl;
  if ((arg[0] == '\'' && arg[arg.size() - 1] == '\''))
    return 1;
  else if ((arg[0] == '\"' && arg[arg.size() - 1] == '\"'))
    return 2;
  else
    return 0;
}
vector<string> splitArgs(string arg, char del = '\'')
{
  string part = "";
  vector<string> results;
  int Qcount = 0;
  for (int i = 0; i < arg.size(); i++)
  {
    if (part == " " && arg[i] == ' ' && part.size() == 1)
    {
      continue;
    }
    if (arg[i] == del && (arg[i + 1] == ' ' || arg[i + 1] == del) || part == " ")
    {
      results.push_back(part);
      part = "";
    }
    if (arg[i] == del)
    {
      continue;
    }
    if (arg[i] == '\\' and del == '\"')
    {
      if (i + 1 < arg.size() && (arg[i + 1] == '$' || arg[i + 1] == '"' || arg[i + 1] == '\\'))
      {
        part += arg[i + 1];
        i++;
      }
      else
      {
        part += '\\';
      }
    }
    else
    {
      part += arg[i];
    }
  }
  results.push_back(part);
  return results;
}
vector<string> getCommand(string input)
{
  vector<string> tokens(2);
  string command = "";
  int i = 1;
  char Quote = input[0];
  while (input[i] != Quote)
  {
    command += input[i];
    i++;
  }
  // cout << "command : " << command << endl;
  tokens[0] = command;
  i++;
  command = "";
  while (i < input.size())
  {
    command += input[i];
    i++;
  }
  // cout << "args : " << command << endl;
  tokens[1] = command;
  return tokens;
}
vector<string> splitForSpaces(string args)
{
  vector<string> results;
  string part = "";
  for (int i = 0; i < args.size(); i++)
  {
    if (args[i] != ' ')
      part += args[i];
    else
    {
      results.push_back(part);
      part = "";
    }
  }
  results.push_back(part);
  return results;
}
int main()
{
  cout << unitbuf;
  cerr << unitbuf;
  // for (auto it = pathes.begin(); it != pathes.end(); it++)
  //   cout << *it << endl;
  set<string> commands = {"echo", "exit", "type", "pwd", "cd"};
  string input;
  while (true)
  {
    cout << "$ ";
    getline(std::cin, input);
    if (input == "exit 0")
      break;
    bool cQ = (input[0] == '\'' || input[0] == '\"');
    // dashOne = input.find("-1") != string::npos;
    vector<string> tokens = cQ ? getCommand(input) : vector<string>();
    string command = cQ ? tokens[0] : input.substr(0, input.find(" "));
    string arguments = cQ ? tokens[1] : input.substr(input.find(" ") + 1);
    string outPutFile;
    bool inFile = false;
    size_t redirPos = arguments.find(">");
    bool stderr_2 = arguments[redirPos - 1] == '2';
    bool append = arguments[redirPos + 1] == '>';
    // cout << append << endl;
    if (redirPos != string::npos)
    {
      outPutFile = append ? arguments.substr(redirPos + 2) : arguments.substr(redirPos + 1);
      size_t firstChar = outPutFile.find_first_not_of(" ");
      size_t lastChar = outPutFile.find_last_not_of(" ");
      outPutFile = outPutFile.substr(firstChar, lastChar - firstChar + 1);
      if (arguments[redirPos - 1] == '1' || arguments[redirPos - 1] == '2')
        arguments = arguments.substr(0, redirPos - 2);
      else
        arguments = arguments.substr(0, redirPos - 1);
      inFile = true;
    }
    // cout << arguments << endl;
    bool isCommand = true;
    if (command == "echo")
    {
      int containQ = containQuotes(arguments);
      // cout << containQ << endl;
      string output = "";
      if (containQ)
      {
        vector<string> args = containQ == 2 ? splitArgs(arguments, '\"') : splitArgs(arguments);
        for (auto &arg : args)
        {
          // cout<<arg<<endl;
          ofstream out(outPutFile, append ? ios::app : ios::out);
          for (int i = 0; i < arg.size(); i++)
          {
            output += arg[i];
          }
          if (inFile && stderr_2 == false)
          {
            out << output << endl;
            out.close();
          }
          else if (stderr_2)
          {
            try
            {
              cout << output << endl;
            }
            catch (const std::exception &e)
            {
              out << e.what();
              // out.close();
            }
          }
          else
            cout << output;
          output = "";
        }
        if (!inFile)
          cout << endl;
      }
      else
      {
        bool space = false;
        for (int i = 0; i < arguments.size(); i++)
        {
          // if (i != arguments.size() - 1 && arguments[i] == '\\')
          //   output += arguments[i + 1];
          if (i > 0 && arguments[i] == '\\' && arguments[i - 1] == '\\')
            output += arguments[i];
          if (arguments[i] != ' ' && arguments[i] != '\\')
            output += arguments[i];
          if (arguments[i] != ' ' && arguments[i + 1] == ' ')
          {
            // output += arguments[i];
            output += " ";
          }
        }
        //  \'\"example world\"\'
        //  '"example world "'
        if (inFile)
        {
          ofstream out(outPutFile, append ? ios::app : ios::out);
          out << output << endl;
          // out.close();
        }
        else
          cout << output << endl;
        // cout << arguments << endl;
      }
    }
    else if (command == "type")
    {
      if (commands.find(arguments) != commands.end())
      {
        cout << arguments << " is a shell builtin\n";
        isCommand = false;
      }
      else
      {
        string path = get_path(arguments);
        if (path != "")
        {
          cout << arguments << " is " << path << endl;
          isCommand = false;
        }
      }
      if (isCommand)
        cout << arguments << ": not found\n";
    }
    else if (command == "pwd")
    {
      cout << filesystem::current_path().string() << endl;
    }
    else if (command == "cd")
    {
      try
      {
        if (arguments.empty() || arguments == "~")
        {
          char *home = getenv("HOME");
          if (home)
          {
            filesystem::current_path(home);
          }
          else
          {
            cerr << "cd: HOME not set" << endl;
          }
        }
        else if (filesystem::exists(arguments) && filesystem::is_directory(arguments))
        {
          filesystem::current_path(arguments);
        }
        else
        {
          cerr << "cd: " << arguments << ": No such file or directory" << endl;
        }
      }
      catch (const filesystem::filesystem_error &e)
      {
        cerr << "cd: " << arguments << ": No such file or directory" << endl;
      }
    }
    else if (command == "cat")
    {
      // cout << "cat command entered\n";
      int containQ = containQuotes(arguments);
      vector<string> files = containQ == 2 ? splitArgs(arguments, '\"') : splitArgs(arguments);
      bool all_exists = true;
      bool firstFile = true;
      fstream fileOut;
      string line;
      if (inFile)
      {
        files = splitForSpaces(arguments);
        ofstream out(outPutFile, append ? ios::app : ios::out);
        for (const auto &file : files)
        {
          if (file == " ")
            continue;
          fileOut.open(file);
          if (!fileOut.is_open())
          {
            if (!stderr_2)
            {
              cerr << "cat: " << file << ": No such file or directory" << endl;
              all_exists = false;
              break;
            }
            else
            {
              out << "cat: " << file << ": No such file or directory" << endl;
            }
            continue;
          }
          while (getline(fileOut, line))
          {
            if (!stderr_2)
            {
              out << line;
            }
            else
              cout << line << endl;
          }
          fileOut.close();
          firstFile = false;
          // cout << "output : " << output << endl;
        }
        // cout << endl;
        // break;
      }
      // cout << "file size :" << files.size() << endl;
      if (!inFile)
      {
        for (const auto &file : files)
        {
          // cout << "file :" << file << endl;
          if (file == " ")
            continue;
          fileOut.open(file);
          if (!fileOut.is_open())
          {
            cerr << "cat: " << file << ": No such file or directory" << endl;
            continue;
          }
          while (getline(fileOut, line))
          {
            if (!containQ)
            {
              cout << line << endl;
            }
            else
              cout << line;
          }
          fileOut.close();
          fileOut.clear();
        }
      }
      if (containQ)
        cout << endl;
    }
    else if (input[0] == '\'' || input[0] == '\"')
    {
      try
      {
        string resolvedPath = get_path(command, cQ, string(1, input[0]));
        string fullExe = resolvedPath + " " + arguments;
        // cout << resolvedPath << endl;
        // if (is_exe(resolvedPath))
        // {
        int result = system(fullExe.c_str());
        if (result != 0)
        {
          cerr << "Error: Command execution failed." << endl;
        }
        // }
        // else
        // {
        //   cout << "hhhhhhhhhh: " << fullExe.c_str() << endl;
        //   cerr << "Error: " << command << " is not executable." << endl;
        // }
      }
      catch (const filesystem::filesystem_error &e)
      {
        cerr << "Error: " << e.what() << endl;
      }
    }
    else if (is_exe(command))
    {
      string fullExe = get_basename(get_path(command)) + " " + arguments;
      if (inFile)
      {
        // cout << "infile .... ";
        ofstream out(outPutFile, append ? ios::app : ios::out);
        string cmdWithRedirect = fullExe + " 2>&1";
        FILE *pipe = stderr_2 ? popen(cmdWithRedirect.c_str(), "r") : popen(fullExe.c_str(), "r");
        // cout<<"pipe : " << pipe << endl;
        if (!pipe)
        {
          cerr << "Error: Failed to execute command: " << fullExe << endl;
          out.close();
          return 0;
        }
        char buffer[128];
        // cout << *fgets(buffer, sizeof(buffer), pipe) << endl;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
          out << buffer;
          // cout << "here _1 " << endl;
        }
        pclose(pipe);
        // out.close();
      }
      else
      {
        // cout << "not infile=========";
        system(fullExe.c_str());
      }
    }
    else
      cout << input << ": command not found\n";
  }
}