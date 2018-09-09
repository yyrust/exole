#ifndef EXOLE_FILE_NAME_COMPLETER_H
#define EXOLE_FILE_NAME_COMPLETER_H

#include <string>
#include <vector>

namespace exole {

enum FileType {
    FT_UNKNOWN = 0,
    FT_REGULAR_FILE = 1,
    FT_DIRECTORY = 2,
    FT_ALL_TYPES = FT_REGULAR_FILE | FT_DIRECTORY,
};

class FileNameCompletionItem
{
public:
    FileNameCompletionItem(const std::wstring &value, FileType type)
    : value_(value)
    , type_(type)
    {}

    const std::wstring &value() const { return value_; }
    FileType type() const { return type_; }
private:
    std::wstring value_;
    FileType type_;
};

class FileNameCompleter
{
public:
    static std::vector<FileNameCompletionItem> complete(const wchar_t *token, size_t len, std::wstring &completion, int types = FT_ALL_TYPES);
    static std::vector<FileNameCompletionItem> list_files(const std::string &dir_name, int types = FT_ALL_TYPES);
};

} // namespace exole

#endif // EXOLE_FILE_NAME_COMPLETER_H
