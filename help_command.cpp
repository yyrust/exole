#include "help_command.h"
#include "application.h"
#include "console.h"
#include "token_parser.h"

namespace exole {

HelpCommand::HelpCommand(const std::wstring &name)
: Command(name)
{
    set_usage(L"help [command [sub-command]]: show help message");
}

void HelpCommand::run(Application &app, int argc, const wchar_t **argv)
{
    if (argc == 0) {
        app.current_console()->show_help(app);
        return;
    }

    // Example: "help c1 c2 c3 c4", finally *console* points to c3 and *sub_cmd* points to c4.
    Console *console = app.current_console();
    Command *sub_cmd = nullptr;
    std::wstring arg;
    while (argc > 0) {
        arg = argv[0];
        sub_cmd = console->command_manager().find_command(arg);
        if (sub_cmd == nullptr) {
            fprintf(stderr, "ERROR: command '%ls' not found\n", arg.c_str());
            return;
        }
        if (argc > 1) {
            Console *sub_console = dynamic_cast<Console *>(sub_cmd);
            if (sub_console == nullptr) {
                fprintf(stderr, "ERROR: command '%ls' has no sub commands\n", arg.c_str());
                return;
            }
            console = sub_console;
        }
        argv++;
        argc--;
    }

    std::wstring usage = sub_cmd->usage();
    printf("%ls\n", usage.c_str());

    Console *sub_console = dynamic_cast<Console *>(sub_cmd);
    if (sub_console != nullptr) {
        sub_console->show_help(app);
    }
}

std::vector<CompletionItem> HelpCommand::auto_complete(Application & app,
            const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion)
{
    completion.clear();
    TokenParser parser;
    parser.parse(line, len, cursor);
    const auto &cursor_info = parser.get_cursor_info();
    std::vector<CompletionItem> result;

    // Example: "help c1 c2 c3", finally *console* points to c1
    //                    ^
    //                 cursor
    Console *console = app.current_console();
    std::wstring arg;
    for (size_t i = 0; i < cursor_info.token_index; i++) {
        const Token &tok = parser.tokens()[i];
        Command *sub_cmd = console->command_manager().find_command(tok.value());
        if (sub_cmd == nullptr) { // command not found
            return result;
        }
        Console *sub_console = dynamic_cast<Console *>(sub_cmd);
        if (sub_console == nullptr) { // not a console
            return result;
        }
        console = sub_console;
    }
    auto commands = console->command_manager().match_by_prefix(cursor_info.prefix, completion);
    if (!commands.empty()) {
        result.reserve(commands.size());
        for (auto c : commands) {
            result.push_back(CompletionItem(c->name(), true));
        }
    }
    return result;
}

} // namespace exole
