#ifndef FOLDER_H
#define FOLDER_H

#include <string>
#include <vector>

namespace espnix
{
    class File;
    class Folder;

    class Folder
    {
    public:
        std::string name;
        std::vector<File *> files;
        std::vector<Folder *> folders;
        Folder *parent;
        Folder *current;
        int owner;
        int permissions;
        long creationDate;

        Folder();
        void AddFile(File *file);
        void AddFolder(Folder *folder);
        std::vector<void *> ListContent();
        void RemoveFile(std::string filename);
        void RemoveFolder(std::string foldername);

        ~Folder();
    };
}

#endif