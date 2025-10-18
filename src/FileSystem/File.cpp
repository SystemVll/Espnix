#include <string>
#include <cstring>

#include "File.h"

namespace espnix
{
    File::File()
    {
        this->permissions = 0777;
        this->content = nullptr;
        this->size = 0;
    }

    std::string File::Read()
    {
        if (this->content)
        {
            return std::string(this->content);
        }
        return "";
    }

    void File::Write(std::string data)
    {
        this->Remove();
        this->content = static_cast<char*>(malloc(data.size() + 1));
        if (this->content)
        {
            strcpy(this->content, data.c_str());
            this->size = data.size();
        }
        else
        {
            this->size = 0;
        }
    }

    void File::Remove()
    {
        free(this->content);
        this->content = nullptr;
        this->size = 0;
    }
}