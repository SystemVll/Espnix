#ifndef ESPNIX_NANOCOMMAND_H
#define ESPNIX_NANOCOMMAND_H

#include <Shell/Commands/ICommand.h>

class EvimCommand : public ICommand
{
public:
    void Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output) override;
};


#endif