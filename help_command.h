#ifndef EXOLE_HELP_COMMAND_H
#define EXOLE_HELP_COMMAND_H

#include "command.h"

namespace exole {

class HelpCommand : public Command
{
public:
    HelpCommand(const std::wstring &name = L"help");
    void run(Application &app, int argc, const wchar_t **argv) override;

    std::vector<CompletionItem> auto_complete(Application & app,
            const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion) override;
};

} // namespace exole

#endif // EXOLE_HELP_COMMAND_H
