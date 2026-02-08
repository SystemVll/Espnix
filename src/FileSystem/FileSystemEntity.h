#ifndef FILESYSTEM_ENTITY_H
#define FILESYSTEM_ENTITY_H

#include <string>

namespace espnix
{
    class Folder;

    enum class EntityType {
        FILE,
        FOLDER
    };

    class FileSystemEntity
    {
    public:
        std::string name;
        Folder *parent;
        int owner;
        int permissions;
        long creationDate;
        EntityType type;

        FileSystemEntity(EntityType entityType);
        virtual ~FileSystemEntity() = default;

        // Virtual methods for polymorphic behavior
        virtual std::string GetDisplayName() const = 0;
        virtual size_t GetSize() const = 0;
    };
}

#endif
