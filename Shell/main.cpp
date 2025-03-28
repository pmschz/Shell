#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <set>
#include <algorithm>

bool starts_with(const std::string& str, const std::string& prefix) {
  return str.find(prefix) == 0;
}

const std::string CMD_EXIT = "exit";
const std::string CMD_ECHO = "echo";
const std::string CMD_TYPE = "type";
const std::string CMD_PWD = "pwd";
const std::string CMD_CD = "cd";
std::string WORKING_DIR = "";
const std::unordered_set<std::string> AVAILABLE_COMMANDS = {
  CMD_EXIT, CMD_ECHO, CMD_TYPE, CMD_PWD, CMD_CD
};
std::vector<std::string> split(std::string& s, char by) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = s.find(by, start);
  while (end != std::string::npos) {
    result.push_back(s.substr(start, end - start));
    start = end + 1;
    end = s.find(by, start);
  }
  result.push_back(s.substr(start));
  return result;
}
std::vector<std::string> get_paths(std::string& path) {
  return split(path, ':');
}
std::string normalize_quoted(std::string& input) {
  std::string result;
  bool has_single_quote = false;
  bool has_double_quote = false;
  bool after_backslash = false;
  for (auto& ch : input) {
    if (after_backslash) {
      result += ch;
      after_backslash = false;
      continue;
    }
    if (has_single_quote) {
      if (ch == '\'') {
        has_single_quote = false;
      } else {
        result += ch;
      }
    } else if (has_double_quote) {
      if (ch == '\"') {
        has_double_quote = false;
      } else if (ch == '\\') {
        after_backslash = true;
      } else {
        result += ch;
      }
    } else {
      if (ch == ' ' && result.back() == ' ') {
        continue;
      } else if (ch == '\\') {
        after_backslash = true;
      } else if (ch == '\'') {
        has_single_quote = true;
      } else if (ch == '\"') {
        has_double_quote = true;
      } else {
        result += ch;
      }
    }
  }
  return result;
}
std::string normalize_path(std::string& path) {
  auto path_items = split(path, '/');
  std::vector<std::string> result;
  for (auto& path_item : path_items) {
    if (path_item == "") {
      continue;
    } else if (path_item == ".") {
      continue;
    } else if (path_item == "..") {
      if (!result.empty()) {
        result.pop_back();
      }
    } else {
      result.push_back(path_item);
    }
  }
  std::string result_path;
  for (auto& path_item : result) {
    result_path += "/";
    result_path += path_item;
  }
  return result_path;
}
void handle_echo(std::string& input) {
  auto arg = input.substr(CMD_ECHO.size() + 1);
  size_t redirect_pos = arg.find('>');
  if (redirect_pos != std::string::npos) {
    if (arg[redirect_pos-1] == '1' && arg[redirect_pos+1] == '>') {
      std::string path = arg.substr(redirect_pos+3);
      std::fstream file(path, std::ios::app);
      if (arg[redirect_pos-1] == '1') {
        redirect_pos--;
      }
      std::string raw = arg.substr(0, redirect_pos);
      file << normalize_quoted(raw) << std::endl;
      file.close();
    } else if (arg[redirect_pos-1] == '2') {
      std::string path = arg.substr(redirect_pos+2);
      std::fstream file(path, std::ios::out);
      std::string raw = arg.substr(0, redirect_pos-1);
      std::cout << normalize_quoted(raw) << std::endl;
      file.close();
    } else {
      std::string path = arg.substr(redirect_pos+2);
      std::fstream file(path, std::ios::out);
      if (arg[redirect_pos-1] == '1') {
        redirect_pos--;
      }
      std::string raw = arg.substr(0, redirect_pos);
      file << normalize_quoted(raw) << std::endl;
      file.close();
    }
  } else {
    std::cout << normalize_quoted(arg) << std::endl;
  }
}
void handle_type(std::string& input, std::vector<std::string>& paths) {
  auto cmd = input.substr(CMD_TYPE.size() + 1);
  if (AVAILABLE_COMMANDS.count(cmd)) {
    std::cout << cmd << " is a shell builtin" << std::endl;
    return;
  }
  for (auto& path : paths) {
    std::string full_cmd_path = path + "/" + cmd;
    if (std::filesystem::exists(full_cmd_path)) {
      std::cout << cmd << " is " << full_cmd_path << std::endl;
      return;
    }
  }
  std::cout << cmd << ": not found" << std::endl;
}
void handle_pwd() {
  std::cout << WORKING_DIR << std::endl;
}
void handle_cd(std::string& input) {
  auto path = input.substr(CMD_CD.size() + 1);
  if (starts_with(path, "/")) {
    if (!std::filesystem::exists(path)){
      std::cout << "cd: " << path << ": No such file or directory" << std::endl;
      return;
    }
    WORKING_DIR = path;
  } else if (path == "~") {
    std::string home_path;
    if (const char* env_p = std::getenv("HOME")) {
      home_path = env_p;
    }
    WORKING_DIR = home_path;
  } else {
    auto temp_working_dir = WORKING_DIR + "/" + path;
    WORKING_DIR = normalize_path(temp_working_dir);
  }
}
void handle_run(std::string& cmd_line, std::vector<std::string>& paths) {
  auto args = split(cmd_line, ' ');
  std::string cmd = args[0];
  if (cmd.front() == '\'' ) {
    size_t end_pos = cmd_line.find('\'', 1);
    cmd = cmd_line.substr(1, end_pos - 1);
  }
  if (cmd.front() == '\"' ) {
    size_t end_pos = cmd_line.find('\"', 1);
    cmd = cmd_line.substr(1, end_pos - 1);
  }
  for (auto& path : paths) {
    std::string full_cmd_path = path + "/" + cmd;
    if (std::filesystem::exists(full_cmd_path)) {
      system(cmd_line.c_str());
      return;
    }
  }
  std::cout << cmd << ": not found" << std::endl;
}
std::string find_common_prefix(const std::set<std::string>& matches) {
    if (matches.empty()) return "";
    if (matches.size() == 1) return *matches.begin();
    const std::string& first = *matches.begin();
    size_t min_len = first.length();
    for (const auto& str : matches) {
        min_len = std::min(min_len, str.length());
    }
    
    std::string result;
    for (size_t i = 0; i < min_len; i++) {
        char current = first[i];
        for (const auto& str : matches) {
            if (str[i] != current) {
                return result;
            }
        }
        result += current;
    }
    
    return result;
}
std::string get_input_with_completion(const std::vector<std::string>& paths) {
  std::string input;
  char c;
  struct termios old_settings, new_settings;
  tcgetattr(STDIN_FILENO, &old_settings);
  new_settings = old_settings;
  new_settings.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
  std::cout << "$ ";
  bool is_tab_once = false;
  while (true) {
    c = getchar();
    // TAB
    if (c == '\t') {
      std::set<std::string> matches;
      if (starts_with(CMD_ECHO, input)) {
        matches.insert(CMD_ECHO);
      }
      if (starts_with(CMD_EXIT, input)) {
        matches.insert(CMD_EXIT);
      }
      if (matches.size() == 1) {
        while (!input.empty()) {
            std::cout << "\b \b";
            input.pop_back();
        }
        input = *matches.begin();
        input += " ";
        std::cout << input;
        continue;
      }
      for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::string filename = entry.path().filename().string();
            if (starts_with(filename, input)) {
              matches.insert(filename);
            }
        }
      }
      if (matches.size() == 0) {
        std::cout << '\a';
      } else if (matches.size() == 1) {
        while (!input.empty()) {
            std::cout << "\b \b";
            input.pop_back();
        }
        input = *matches.begin();
        input += " ";
        std::cout << input;
      } else if (matches.size() > 1 && !is_tab_once) {
        std::string common = find_common_prefix(matches);
        if (common.length() > input.length()) {
            while (!input.empty()) {
                std::cout << "\b \b";
                input.pop_back();
            }
            input = common;
            std::cout << input;
        } else {
            is_tab_once = true;
            std::cout << '\a';
        }
      } else if (matches.size() > 1 && is_tab_once) {
        std::cout << "\n";
        for (const auto& match : matches) {
            std::cout << match << "  ";
        }
        std::cout << "\n$ " << input;
      }
    // ENTER
    } else if (c == '\n') {
      std::cout << std::endl;
      break;
    // BACKSPACE
    } else if (c == 127 || c == '\b') {
      if (!input.empty()) {
          input.pop_back();
          std::cout << "\b \b";
      }
    // DEFAULT
    } else {
        input += c;
        std::cout << c;
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
  return input;
}
int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  WORKING_DIR = std::filesystem::current_path().string();
  std::vector<std::string> paths;
  if (const char* env_p = std::getenv("PATH")) {
      std::string path = env_p;
      paths = get_paths(path);
  }
  while (true) {
    std::string input = get_input_with_completion(paths);
    if (input == "exit 0") {
      break;
    } else if (starts_with(input, CMD_ECHO)) {
      handle_echo(input);
    } else if (starts_with(input, CMD_TYPE)) {
      handle_type(input, paths);
    } else if (starts_with(input, CMD_PWD)) {
      handle_pwd();
    } else if (starts_with(input, CMD_CD)) {
      handle_cd(input);
    } else {
      handle_run(input, paths);
    }
  }
  return EXIT_SUCCESS;
}