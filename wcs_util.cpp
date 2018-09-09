#include "wcs_util.h"
#include <cerrno>
#include <cassert>
#include <cstring>

namespace exole {

std::string wcs_to_mbs(const wchar_t *wstr, int wlen)
{
    int dstlen = 4 * wlen;
    std::vector<char> dst(dstlen);
    size_t n = ::wcsnrtombs(dst.data(), &wstr, wlen, dstlen, nullptr);
    if (n == size_t(-1)) { // failed
        char buf[64];
        strerror_r(errno, buf, 64);
        fprintf(stderr, "error[%d]: %s\n", errno, buf);
        return std::string();
    }
    std::string str(dst.data(), n);
    return str;
}

std::string wcs_to_mbs(const std::wstring &wstr)
{
    return wcs_to_mbs(wstr.c_str(), wstr.size());
}

std::string wcs_to_mbs(const wchar_t *wstr)
{
    return wcs_to_mbs(wstr, wcslen(wstr));
}

std::wstring mbs_to_wcs(const char *mbstr, int len)
{
    int dstlen = len;
    wchar_t *dst = new wchar_t [dstlen];
    size_t n = ::mbsnrtowcs(dst, &mbstr, len, dstlen, nullptr);
    std::wstring str(dst, n);
    delete [] dst;
    return str;
}

std::wstring mbs_to_wcs(const char *mbstr)
{
    return mbs_to_wcs(mbstr, strlen(mbstr));
}

std::wstring mbs_to_wcs(const std::string &mbstr)
{
    return mbs_to_wcs(mbstr.c_str(), mbstr.size());
}

template<typename String>
String common_prefix_impl(const String &lhs, const String &rhs)
{
    size_t size = std::min(lhs.size(), rhs.size());
    size_t i;
    for (i = 0; i < size && lhs[i] == rhs[i]; i++)
        ;
    return lhs.substr(0, i);
}

std::wstring common_prefix(const std::wstring &lhs, const std::wstring &rhs)
{
    return common_prefix_impl(lhs, rhs);
}

std::string common_prefix(const std::string &lhs, const std::string &rhs)
{
    return common_prefix_impl(lhs, rhs);
}

} // namespace exole
