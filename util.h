#ifndef EXOLE_UTIL_H
#define EXOLE_UTIL_H

namespace exole {

template <typename T, typename Release>
class ScopedGuard
{
    T resource_;
    Release release_;
public:
    ScopedGuard(T resource, Release release)
    : resource_(resource)
    , release_(release)
    {}
    ~ScopedGuard()
    {
        release_(resource_);
    }
};

template <typename T, typename Release>
ScopedGuard<T, Release> MakeScopeGuard(T resource, Release release)
{
    return ScopedGuard<T, Release>(resource, release);
}
    
} // exole

#define EXOLE_SCOPE_EXIT(RES, ON_EXIT)\
    auto _SCOPE_EXIT_##__LINE__ = ::exole::MakeScopeGuard(RES, ON_EXIT)

#endif // EXOLE_UTIL_H
