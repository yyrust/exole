#include "arguments.h"
#include <cstdlib>
#include <cstring>
#include <string>

namespace exole {
namespace detail {

Arguments::Arguments()
: argc_(0)
, argv_(nullptr)
, frozen_(false)
{}

Arguments::~Arguments()
{
    clear();
}

void Arguments::clear()
{
    for (int i = 0; i < argc_; i++) {
        ::free(argv_[i]);
    }
    delete [] argv_;
    argv_ = nullptr;
    argc_ = 0;
}

void Arguments::set(int argc, const wchar_t **argv)
{
    if (frozen_)
        return;
    if (argc < 0) // invalid argument
        return;
    clear();
    if (argc == 0)
        return;
    argc_ = argc;
    argv_ = new wchar_t *[argc_];
    for (int i = 0; i < argc; i++) {
        argv_[i] = wcsdup(argv[i]);
    }
}

} // namespace detail
} // namespace exole
