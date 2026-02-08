#include "FileSystemEntity.h"

namespace espnix
{
    FileSystemEntity::FileSystemEntity(EntityType entityType)
        : type(entityType), parent(nullptr), owner(0), permissions(0755), creationDate(0)
    {
    }
}
