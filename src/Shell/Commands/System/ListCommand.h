#ifndef LIST_COMMAND_H
#define LIST_COMMAND_H

#include <vector>
#include <string>

#include <Shell/Commands/ICommand.h>

class File;
class Terminal;

class ListCommand : public ICommand
{
public:
    void Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output) override;
};

#endif