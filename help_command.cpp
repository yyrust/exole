#include "help_command.h"
#include "application.h"
#include "console.h"

namespace exole {

HelpCommand::HelpCommand(const std::wstring &name)
: Command(name)
{
    set_usage(L"help : show help message");
}

void HelpCommand::run(Application &app, int /*argc*/, const wchar_t ** /*argv*/)
{
    app.current_console()->show_help(app);
}

} // namespace exole
