#ifndef EXOLE_COMPLETION_H
#define EXOLE_COMPLETION_H

#include <string>

namespace exole {

template <typename String>
class BasicCompletionItem
{
public:
    BasicCompletionItem(const String &value, bool is_complete)
    : value_(value)
    , is_complete_(is_complete)
    {}
    const String &value() const { return value_; }
    bool is_complete() const { return is_complete_; }
private:
    String value_;
    bool is_complete_;

};

using CompletionItem = BasicCompletionItem<std::wstring>;

} // namespace exole

#endif // EXOLE_COMPLETION_H
