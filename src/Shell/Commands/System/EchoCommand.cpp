#include "EchoCommand.h"
#include <Terminal/Terminal.h>
#include <IO/FileDescriptor.h>

void EchoCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if (i > 0)
        {
            output->write(" ", 1);
        }
        output->write(args[i].c_str(), args[i].size());
    }

    output->write("\n", 1);
}