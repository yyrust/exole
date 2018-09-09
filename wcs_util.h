#ifndef EXOLE_WCS_UTIL_H
#define EXOLE_WCS_UTIL_H

#include <string>
#include <cstring>
#include <vector>

namespace exole {

std::string wcs_to_mbs(const wchar_t *wstr, int wlen);
std::string wcs_to_mbs(const wchar_t *wstr);
std::string wcs_to_mbs(const std::wstring &wstr);

std::wstring mbs_to_wcs(const char *mbstr, int len);
std::wstring mbs_to_wcs(const char *mbstr);
std::wstring mbs_to_wcs(const std::string &mbstr);

std::wstring common_prefix(const std::wstring &lhs, const std::wstring &rhs);
std::string common_prefix(const std::string &lhs, const std::string &rhs);

inline int StringCmp(const wchar_t *lhs, const wchar_t *rhs, size_t len) { return std::wcsncmp(lhs, rhs, len); }
inline int StringCmp(const char *lhs, const char *rhs, size_t len) { return std::strncmp(lhs, rhs, len); }

template <typename Iterator, typename GetName, typename String>
std::vector<typename Iterator::value_type>
match_by_prefix(Iterator first, Iterator last, GetName get_name, const String &prefix, String &completion)
{
    using Item = typename Iterator::value_type;
    using Name = decltype(get_name(*first));
    completion.clear();
    std::vector<Item> result;
    String max_common_prefix;
    for (Iterator it = first; it != last; ++it) {
        Name name = get_name(*it);
        if (name.size() >= prefix.size() &&
                0 == StringCmp(name.data(), prefix.data(), prefix.size())) {
            if (result.empty()) {
                max_common_prefix = name;
            } else {
                max_common_prefix = common_prefix(max_common_prefix, name);
            }
            result.push_back(*it);
        }
    }
    if (!result.empty()) {
        // assert(max_common_prefix.size() >= prefix.size());
        completion = max_common_prefix.substr(prefix.size());
    }
    return result;
}

template <typename Item, typename GetName, typename String>
std::vector<Item> match_by_prefix(const std::vector<Item> &candidates,
                                  GetName get_name, const String &prefix,
                                  String &completion)
{
    return match_by_prefix(
        candidates.begin(), candidates.end(), get_name, prefix, completion);
}

} // namespace exole

#endif // EXOLE_WCS_UTIL_H
