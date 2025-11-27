#include "FileSystem.h"

FileSystemElement::FileSystemElement(const std::string& name) : name(name) {}

File::File(const std::string& name, size_t size) : FileSystemElement(name), file_size(size) {}

void File::displayInfo(int indent) const {
    std::cout << std::string(indent, ' ') << "[File] " << name << " size: " << file_size << std::endl;
}

size_t File::getSize() const { return file_size; }

std::string File::getName() const { return name; }

void File::setName(std::string new_name) { name = new_name; }

FileSystemElement* File::getParent() const { return parent; }

void File::setParent(FileSystemElement* parent) { this->parent = parent; }

FileSystemElement::Type File::getType() const { return Type::FILE; };

std::string File::getPath() const { return parent->getPath() + "/" + getName(); }

std::vector<std::unique_ptr<FileSystemElement>>::iterator
Folder::searchLocalIter(std::string name) {
    auto it = find_if(elements.begin(), elements.end(),
        [&name](const auto& elem) { return elem->getName() == name; });
    return it;
}

Folder::Folder(const std::string& name) : FileSystemElement(name) {}

void Folder::displayInfo(int indent) const {
    std::cout << std::string(indent, ' ') << "[Folder] " << name << std::endl;
    for (const auto& elem : elements)
        elem->displayInfo(indent + 3);
}

size_t Folder::getSize() const {
    size_t total_size = 0;
    for (const auto& elem : elements)
        total_size += elem->getSize();
    return total_size;
}

std::string Folder::getName() const { return name; }

void Folder::setName(std::string new_name) { name = new_name; }

FileSystemElement* Folder::getParent() const { return parent; }

void Folder::setParent(FileSystemElement* parent) { this->parent = parent; }

FileSystemElement::Type Folder::getType() const { return Type::FOLDER; };

std::string Folder::getPath() const {
    if (parent) return parent->getPath() + "/" + getName();
    return "/" + getName();
}

Folder* Folder::createFolder(std::string name) {
    auto new_folder = std::make_unique<Folder>(name);
    Folder* raw_ptr = new_folder.get();
    raw_ptr->setParent(this);
    elements.push_back(std::move(new_folder));
    return raw_ptr;
}

File* Folder::createFile(std::string name, size_t size) {
    auto new_file = std::make_unique<File>(name, size);
    File* raw_ptr = new_file.get();
    raw_ptr->setParent(this);
    elements.push_back(std::move(new_file));
    return raw_ptr;
}

FileSystemElement* Folder::searchLocal(const std::string& name) {
    auto it = searchLocalIter(name);
    if (it != elements.end()) return it->get();
    else return nullptr;
}

FileSystemElement* Folder::deepSearch(const std::string& name) {
    for (auto& element : elements) {
        switch (element->getType()) {
        case Type::FILE:
            if (name == element->getName()) { return element.get(); }
            break;
        case Type::FOLDER:
            if (name == element->getName()) { return element.get(); }
            auto ptr = static_cast<Folder*>(element.get())->deepSearch(name);
            if (ptr) return ptr;
            break;
        }
    }
    return nullptr;
}

bool Folder::removeElement(const std::string& name) {
    auto it = searchLocalIter(name);
    if (it != elements.end()) {
        elements.erase(it);
        return true;
    }
    return false;
}

void Folder::calculateElements(int& fileCount, int& folderCount) {
    for (auto& element : elements) {
        switch (element->getType()) {
        case Type::FILE:
            fileCount++; break;
        case Type::FOLDER:
            folderCount++;
            static_cast<Folder*>(element.get())->calculateElements(fileCount, folderCount);
            break;
        }
    }
}


FileSystem::FileSystem() {
    root = std::make_unique<Folder>("root");
    current_folder = root.get();
    initStruct();
}

Folder* FileSystem::getCurrentFolder() { return current_folder; }

void FileSystem::CommandProcessor(const std::string& user_input) {
    std::vector<std::string> cmd_input = splitStr(user_input, ' ');
    if (cmd_input.empty()) return;
    std::string command = cmd_input[0];

    // Обработка команды /s
    if (command == "/s") {
        displayCurrentDirectory();
        return;
    }

    Commands cmd = parseCommand(command);
    cmd_input.erase(cmd_input.begin());
    switch (cmd) {
    case Commands::CD:
        if (!cmd_input.empty()) navigateToPath(cmd_input[0]);
        break;
    case Commands::TREE:
        current_folder->displayInfo();
        break;
    default:
        std::cout << "Неизвестная команда!\n";
        break;
    }
}

FileSystem::Commands FileSystem::parseCommand(const std::string& command) {
    if (command == "cd") return Commands::CD;
    if (command == "ls") return Commands::LS;
    if (command == "tree") return Commands::TREE;
}

void FileSystem::navigateToPath(const std::string& path) {
    if (!path.empty()) {
        if (path[0] == '/') current_folder = navigateFromRoot(path);
        else current_folder = navigateFromCurrent(path);
    }
}

Folder* FileSystem::navigateFromRoot(const std::string& path) {
    Folder* end_folder = root.get();
    std::vector<std::string> parts = splitStr(path, '/');
    for (const std::string& part : parts) {

        if (part == "root") continue;
        Folder* temp = dynamic_cast<Folder*>(end_folder->searchLocal(part));
        if (!temp) {
            std::cout << "! - " << part << '\n';
            return current_folder;
        }
        end_folder = temp;
    }
    return end_folder;
}

Folder* FileSystem::navigateFromCurrent(const std::string& path) {

    Folder* end_folder = current_folder;
    std::vector<std::string> parts = splitStr(path, '/');

    for (const std::string& part : parts) {
        if (part == "..") {
            if (end_folder->getParent())
                end_folder = dynamic_cast<Folder*>(end_folder->getParent());
        }
        else if (part != ".") {
            Folder* temp = dynamic_cast<Folder*>(end_folder->searchLocal(part));
            if (!temp) {
                std::cout << "! - " << part << '\n';
                return current_folder;
            }
            end_folder = temp;
        }
    }
    return end_folder;
}

std::vector<std::string> FileSystem::splitStr(const std::string& str, char sym) {

    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    while (std::getline(ss, part, sym)) {
        if (!part.empty()) parts.push_back(part);
    }
    return parts;
}

void FileSystem::initStruct() {

    Folder* documents = root->createFolder("documents");
    Folder* images = root->createFolder("images");

    documents->createFile("resume.pdf", 2048);
    documents->createFile("contract.docx", 4096);

    Folder* projects = documents->createFolder("projects");
    projects->createFile("main.cpp", 1536);
    projects->createFile("utils.h", 512);
    projects->createFile("README.md", 1024);

    Folder* work = documents->createFolder("work");
    work->createFile("report.xlsx", 3072);
    work->createFile("presentation.pptx", 6144);

    images->createFile("photo.jpg", 10240);
    images->createFile("screenshot.png", 8192);

    Folder* vacations = images->createFolder("vacations");
    vacations->createFile("beach.jpg", 15360);
    vacations->createFile("mountain.png", 12288);

    Folder* screenshots = images->createFolder("screenshots");
    screenshots->createFile("game.png", 7168);
    screenshots->createFile("app.png", 6144);
}
void FileSystem::displayCurrentDirectory() {
    std::cout << "Содержимое текущей директории " << current_folder->getPath() << ":\n";

    int fileCount = 0;
    int folderCount = 0;
    current_folder->calculateElements(fileCount, folderCount);

    std::cout << "Папок: " << folderCount << ", Файлов: " << fileCount << "\n";
    current_folder->displayInfo(0);
}