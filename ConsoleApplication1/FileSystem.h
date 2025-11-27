#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <sstream> 

/* == FileSystemElement START == */

class FileSystemElement {
protected:
    std::string name;
    FileSystemElement* parent = nullptr;
    FileSystemElement(const std::string&);
    enum class Type { FILE, FOLDER };
    virtual ~FileSystemElement() = default;
    virtual void displayInfo(int) const = 0;
    virtual size_t getSize() const = 0;
    virtual std::string getName() const = 0;
    virtual void setName(std::string) = 0;
    virtual FileSystemElement* getParent() const = 0;
    virtual void setParent(FileSystemElement*) = 0;
    virtual Type getType() const = 0;
    virtual std::string getPath() const = 0;
};

/* == FileSystemElement END == */


/* == File START == */

class File : public FileSystemElement {
private:
    size_t file_size;
public:
    File(const std::string&, size_t size = 0);
    void displayInfo(int indent = 0) const override;
    size_t getSize() const override;
    std::string getName() const override;
    void setName(std::string) override;
    FileSystemElement* getParent() const override;
    void setParent(FileSystemElement*) override;
    FileSystemElement::Type getType() const override;
    std::string getPath() const override;
};

/* == File END == */

/* == Folder START == */

class Folder : public FileSystemElement {
private:
    std::vector<std::unique_ptr<FileSystemElement>> elements;
    std::vector<std::unique_ptr<FileSystemElement>>::iterator searchLocalIter(std::string);

public:
    Folder(const std::string&);
    void displayInfo(int indent = 0) const override;
    size_t getSize() const override;
    std::string getName() const override;
    void setName(std::string) override;
    FileSystemElement* getParent() const override;
    void setParent(FileSystemElement* parent) override;
    FileSystemElement::Type getType() const override;
    std::string getPath() const override;

    Folder* createFolder(std::string);
    File* createFile(std::string, size_t);
    FileSystemElement* searchLocal(const std::string&);
    FileSystemElement* deepSearch(const std::string&);
    bool removeElement(const std::string&);
    void calculateElements(int&, int&);

    friend class FileSystem;
};

/* == Folder END == */

class FileSystem {
private:
    std::unique_ptr<Folder> root;
    Folder* current_folder;
    enum class Commands {
        CD, LS, TREE
    };

public:
    FileSystem();
    Folder* getCurrentFolder();
    void CommandProcessor(const std::string&);

private:
    void initStruct();
    void navigateToPath(const std::string&);
    Folder* navigateFromRoot(const std::string&);
    Folder* navigateFromCurrent(const std::string&);
    std::vector<std::string> splitStr(const std::string&, char);
    Commands parseCommand(const std::string&);
    void displayCurrentDirectory();
};