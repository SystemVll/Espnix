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
        if (this->content && this->size > 0)
        {
            // Use size parameter to support binary data with null bytes
            return std::string(this->content, this->size);
        }
        return "";
    }

    void File::Write(std::string data)
    {
        this->Remove();
        this->size = data.size();
        this->content = static_cast<char*>(malloc(this->size + 1));
        if (this->content)
        {
            // Use memcpy to support binary data (including null bytes)
            memcpy(this->content, data.c_str(), this->size);
            this->content[this->size] = '\0';  // Null terminate for safety
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