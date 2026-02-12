#ifndef SHELL_H
#define SHELL_H

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

class ICommand;
class Terminal;
class Loader;

class Shell
{
private:
    std::map<std::string, std::shared_ptr<ICommand>> commandRegistry;
    std::string prompt;
    FileSystem * fs;

public:
    Terminal *terminal;
    Loader *loader;

    Shell();
    void SetLoader(Loader *sysLoader);
    void Interpret(const std::string &input);
    void Prompt();
};

#endif