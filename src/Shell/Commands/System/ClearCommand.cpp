#include "ClearCommand.h"
#include <Terminal/Terminal.h>

void ClearCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    terminal->Clear();
}