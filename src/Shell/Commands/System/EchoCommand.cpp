#include "EchoCommand.h"
#include <Terminal/Terminal.h>

void EchoCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if (i > 0)
        {
            terminal->Write(" ");
        }
        terminal->Write(args[i]);
    }

    terminal->Write("\n");
}