#ifndef CAT_COMMAND_H
#define CAT_COMMAND_H

#include <vector>
#include <string>

#include <Shell/Commands/ICommand.h>

class Terminal;

class CatCommand : public ICommand
{
public:
    void Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output) override;
};

#endif