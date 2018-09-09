#ifndef EXOLE_COMMAND_H
#define EXOLE_COMMAND_H

#include <string>
#include <vector>
#include "command_context.h"
#include "completion.h"

namespace exole {

class Application;

class Command
{
public:
    Command(const std::wstring &name)
    : name_(name)
    {}
    virtual ~Command() {}

    const std::wstring &name() const { return name_; }
    const std::wstring &usage() const { return usage_; }
    void set_usage(const std::wstring &usage) { usage_ = usage; }

    virtual void run(Application &app, int argc, const wchar_t **argv) = 0;

    // Exapmle:
    // If command name is "show"
    // and user input is "show xyz abcdef"
    //                                ^
    //                                |
    //                              cursor
    // then arguments will be:
    //  line: L"xyz abcdef"
    //  len: 10
    //  cursor: L"def"
    //
    // If a token has 3 candidates: ["open", "close", "clone"], the prefix is "c", the return value
    // should be ["close", "clone"] and completion should be "lo" .
    virtual std::vector<CompletionItem> auto_complete(Application & /*app*/,
            const wchar_t * /*line*/, size_t /*len*/, const wchar_t * /*cursor*/, std::wstring &/*completion*/)
    {
        return std::vector<CompletionItem>();
    }
private:
    std::wstring name_;
    std::wstring usage_;
};

} // namespace exole

#endif // EXOLE_COMMAND_H
