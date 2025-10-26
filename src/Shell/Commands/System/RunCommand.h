#ifndef RUN_COMMAND_H
#define RUN_COMMAND_H

#include <vector>
#include <string>

#include <Shell/Commands/ICommand.h>

class Terminal;

class RunCommand : public ICommand
{
public:
    void Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output) override;
};

#endif

