#include "file_name_completer.h"
#include "wcs_util.h"
#include <dirent.h>
#include <cstring>

namespace exole {

std::vector<FileNameCompletionItem> FileNameCompleter::list_files(const std::string &dir_name, int types)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dir_name.c_str())) != NULL) {
        std::vector<FileNameCompletionItem> result;
        while ((ent = readdir (dir)) != NULL) {
            FileType type = FT_UNKNOWN;
            switch (ent->d_type) {
            case DT_REG: type = FT_REGULAR_FILE; break;
            case DT_DIR: type = FT_DIRECTORY; break;
            }
            if ((type & types) == 0) {
                continue;
            }
            // TODO: handle other file types

            if (0 == strcmp(".", ent->d_name) || 0 == strcmp("..", ent->d_name)) {
                continue;
            }
            std::wstring name = mbs_to_wcs(ent->d_name);
            if (type == FT_DIRECTORY) {
                name += '/';
            }
            result.push_back(FileNameCompletionItem(name, type));
        }
        closedir (dir);
        return result;
    } else {
        /* could not open directory */
        return std::vector<FileNameCompletionItem>();
    }
}

std::vector<FileNameCompletionItem> FileNameCompleter::complete(const wchar_t *token, size_t len, std::wstring &completion, int types)
{
    std::string mbs_token = wcs_to_mbs(token, len);
    size_t sep = mbs_token.find_last_of('/');

    std::string dir_name = (sep != mbs_token.npos) ? mbs_token.substr(0, sep+1) : "./";
    std::string file_prefix = (sep != mbs_token.npos) ? mbs_token.substr(sep+1) : mbs_token;
    auto candidates = list_files(dir_name, types);
    std::wstring prefix = mbs_to_wcs(file_prefix);
    return match_by_prefix(candidates,
            [](const FileNameCompletionItem &item) { return item.value(); },
            prefix, completion);
}

} // namespace exole
