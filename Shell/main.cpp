#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
std::vector<std::string> split(std::string& input, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while((pos = input.find(delimiter)) != std::string::npos)
    {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    token = input.substr(0, input.size());
    tokens.push_back(token);
    return tokens;
}
bool isEscapeableInQuotes(char inputChar) {
    return (inputChar == '\\' || inputChar == '$' || inputChar == '"' || inputChar == '\n');
}
std::vector<std::string> createInputTokens(std::string& input) {
    std::vector<std::string> tokens;
    std::string token;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    bool escapeNext = false;
    bool tokensArePaths = false;
    for (size_t i = 0; i < input.size(); i++) {
        char currentChar = input[i];
        if (escapeNext){
            token.push_back(currentChar);
            escapeNext = false;
            continue;
        }
        if (currentChar == '\'') {
            if (inDoubleQuotes) {
                token.push_back(currentChar);
            }
            else {
                inSingleQuotes = !inSingleQuotes;
            }
        }
        else if (currentChar == '\"') {
            if (inSingleQuotes) {
                token.push_back(currentChar);
            }
            if (tokensArePaths && (i == 0 || (i != input.size() - 1 && input[i - 1] == '"'))) {
                // Do not create quotes logic in paths from the wrapping quotes
                continue;
            }
            inDoubleQuotes = !inDoubleQuotes;
        }
        else if (currentChar == ' ') {
            if (inSingleQuotes || inDoubleQuotes) {
                token.push_back(currentChar);
            }
            else {
                if (!token.empty()) {
                    if (tokens.empty()) {
                        if (token == "cat") {
                            tokensArePaths = true;
                        }
                    }
                    tokens.push_back(token);
                    if (tokens.size() > 1) {}
                    token = "";
                }
            }
        }
        else if (currentChar == '\\'){
            // Handle non quoted escape character, ignore it
            if (!inDoubleQuotes && !inSingleQuotes){
                escapeNext = true;
                continue;
            }
            if (inDoubleQuotes){
                if (i < input.size() - 1 && isEscapeableInQuotes(input[i + 1])) {
                    escapeNext = true;
                    continue;
                }
                else if (!tokensArePaths) {
                    token.push_back(currentChar);
                    token.push_back(currentChar);
                }
                else if (tokensArePaths) {
                    token.push_back(currentChar);
                }
            }
            else if (inSingleQuotes){
                token.push_back(currentChar);
                escapeNext = true;
                continue;
            }
        }
        else {
            token.push_back(currentChar);
        }
        escapeNext = false;
    }
    tokens.push_back(token);
    return tokens;
}
bool isExecutable(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path)) {
        auto permissions = std::filesystem::status(path).permissions();
        return (permissions & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
            (permissions & std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
            (permissions & std::filesystem::perms::others_exec) != std::filesystem::perms::none;
    }
    return false;
}
bool isCustomExecutable(const std::filesystem::path& path)
{
    auto permissions = std::filesystem::status(path).permissions();
    return (permissions & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
        (permissions & std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
        (permissions & std::filesystem::perms::others_exec) != std::filesystem::perms::none;
}
void handleEcho(const std::vector<std::string> tokens){
    std::string output = "";
    bool writeToFile = false;
    bool writeToStdOut = true;
    std::string fileName;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == ">" || tokens[i] == "1>") {
            fileName = tokens[i + 1]; // boundary?
            writeToFile = true;
            writeToStdOut = false;
            break;
        }
        output += tokens[i] + " ";
    }
    if (writeToStdOut) {
        std::cout << output << std::endl;
    }
    if (writeToFile) {
        std::ofstream outputFile(fileName);
        if (outputFile.is_open()) {
            outputFile << output << std::endl;
            outputFile.close();
        }
    }
}
std::vector<std::string> getAllPathsFromEnvironment(){
    char const* path = std::getenv("PATH");
    if (path != NULL){
        std::string pathAsString = path;
        return split(pathAsString, ":");
    }
    std::vector<std::string> empty;
    return empty;
}
void handleType(const std::vector<std::string> tokens){
    if (tokens[1] == "echo" ||
        tokens[1] == "type" ||
        tokens[1] == "pwd" ||
        tokens[1] == "exit")
    {
        std::cout << tokens[1] << " is a shell builtin" << std::endl;
    }
    else {
        std::vector<std::string> allPathsFromEnvironment =
                getAllPathsFromEnvironment();
        if (allPathsFromEnvironment.empty()){
            std::cout << tokens[1] << ": not found" << std::endl;
        }
        bool found = false;
        for(auto path : allPathsFromEnvironment){
            std::string fullPath = path + "/" + tokens[1];
            if (std::filesystem::exists(fullPath)){
                found = true;
                std::cout << tokens[1] << " is " << fullPath << std::endl;
                break;
            }
        }
        if (!found){
            std::cout << tokens[1] << ": not found" << std::endl;
        }
    }
}
bool handleExecutable(std::vector<std::string> tokens)
{
    std::vector<std::string> allPathsFromEnvironment =
                getAllPathsFromEnvironment();
    bool isCustomExecutable = false;
    std::string firstToken = tokens[0];
    if (tokens[0].find('\'') != std::string::npos) {
        firstToken = '\"' + tokens[0] + "\" ";
        isCustomExecutable = true;
    }
    else if (tokens[0].find(' ') != std::string::npos){
        firstToken = '\'' + tokens[0] + "' ";
        isCustomExecutable = true;
    }
    for(auto path : allPathsFromEnvironment){
        std::string fullPath = path + "/" + firstToken;
        if (isExecutable(fullPath) || isCustomExecutable){
            std::string command = "";
            for(size_t i = 1; i < tokens.size(); ++i)
            {
                if (tokens[i] == ">" || tokens[i] == "1>") {
                    command += tokens[i] + " ";
                }
                else if (tokens[i].find('\'') != std::string::npos) {
                    command += '\"' + tokens[i] + "\" ";
                }
                else {
                    command += '\'' + tokens[i] + "' ";
                }
            }
            if (tokens[0] == "cat") {
                fullPath = "cat";
            }
            std::string fullCommandStr = fullPath + " " + command;
            const char* fullCommand = fullCommandStr.c_str();
            system(fullCommand);
            return true;
        }
    }
    return false;
}
bool isAbsolutePath(std::string token) {
    return token.length() > 1 && token[0] == '/';
}
bool isRelativePathChild(std::string token) {
    return token.length() > 2 && token[0] == '.' && token[1] == '/';
}
bool isRelativePathParent(std::string token) {
    return token.length() > 2 && token[0] == '.' && token[1] == '.' && token[2] == '/';
}
bool isHomeDirectory(std::string token) {
    return token.length() > 0 && token[0] == '~';
}
std::string goUpNDirectories(std::string currentDirectory, size_t count) {
    std::vector<std::string> allPathSegmentsFromCurrentDirectory = split(currentDirectory, "/");
    std::string newDirectory;
    for(size_t i = 1; i < allPathSegmentsFromCurrentDirectory.size() - count; i++) {
        newDirectory += "/" + allPathSegmentsFromCurrentDirectory[i];
    }
    return newDirectory;
}
int main() {
    std::string currentDirectory = std::filesystem::current_path();
    while (true) {
        // Flush after every std::cout / std:cerr
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;
        // Uncomment this block to pass the first stage
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        std::vector<std::string> tokens = createInputTokens(input);
        if (tokens.empty()){
            continue;
        }
        if (tokens[0] == "exit"){
            break;
        }
        if (tokens[0] == "echo"){
            handleEcho(tokens);
        }
        else if (tokens[0] == "type"){
            handleType(tokens);
        }
        else if (tokens[0] == "pwd")
        {
            std::cout << currentDirectory << std::endl;
        }
        else if (tokens[0] == "cd")
        {
            if (isAbsolutePath(tokens[1])) {
                if (std::filesystem::is_directory(tokens[1]))
                {
                    currentDirectory = tokens[1];
                }
                else
                {
                    std::cout << "cd: " << tokens[1] << ": No such file or directory" << std::endl;
                }
            }
            else if (isRelativePathChild(tokens[1])) {
                std::string directoryToAppend = tokens[1].substr(1, tokens[1].length() - 1);
                std::string testDirectory = currentDirectory + directoryToAppend;
                if (std::filesystem::is_directory(testDirectory))
                {
                    currentDirectory = testDirectory;
                }
                else
                {
                    std::cout << "cd: " << testDirectory << ": No such file or directory" << std::endl;
                }
            }
            else if (isRelativePathParent(tokens[1])) {
                size_t count = 0;
                std::string currentInputDirectory = tokens[1];
                do {
                    count++;
                    currentInputDirectory = currentInputDirectory.substr(3, tokens[1].length() - 3);
                } while (isRelativePathParent(currentInputDirectory));
                currentDirectory = goUpNDirectories(currentDirectory, count);
                if (currentInputDirectory.length() > 1) {
                    currentDirectory += "/" + currentInputDirectory;
                }
            }
            else if (isHomeDirectory(tokens[1])) {
                char const* homeDirectory = std::getenv("HOME");
                if (tokens[1].length() > 1) {
                    std::string directoryToAppend = tokens[1].substr(1, tokens[1].length() - 1);
                    currentDirectory = homeDirectory + directoryToAppend;
                }
                else {
                    currentDirectory = homeDirectory;
                }
            }
            else
            {
                std::cout << "cd: " << tokens[1] << ": No such file or directory" << std::endl;
            }
        }
        else {
            bool wasExecutable = handleExecutable(tokens);
            if (!wasExecutable)
            {
                std::cout << input << ": command not found" << std::endl;
            }
        }
    }
    return 0;
}