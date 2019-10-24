#include "console.h"
#include "application.h"
#include "token_parser.h"
#include "wcs_util.h"
#include "detail/arguments.h"
#include <cassert>

namespace exole {

using detail::Arguments;

Console::Console(std::wstring name)
: Command(name)
, last_arguments_(nullptr)
, repeat_on_empty_(true)
{}

Console::~Console()
{
    delete last_arguments_;
}

void Console::run(Application &app, int argc, const wchar_t **argv)
{
    if (argc == 0) {
        if (app.current_console() != this) {
            // enter this console
            app.enter_console(this);
            on_enter_console(app);
        }
        else if (repeat_on_empty_) {
            // repeat last command
            if (last_arguments_ && last_arguments_->argc() > 0) {
                last_arguments_->freeze();
                run(app, last_arguments_->argc(), last_arguments_->argv());
                last_arguments_->unfreeze();
            }
        }
    }
    else if (argc > 0) {
        if (repeat_on_empty_) {
            // save args as last command
            if (last_arguments_ == nullptr) {
                last_arguments_ = new Arguments();
            }
            last_arguments_->set(argc, argv);
        }

        // check if argv[0] is a subcommand/subconsole
        std::wstring cmd = argv[0];
        Command *command = command_manager().find_command(cmd);
        if (command) {
            // argv[0] is a subcommand/subconsole
            command->run(app, argc-1, argv+1);
        }
        else {
            // argv[0] is not a subcommand/subconsole, may be arbitrary input
            custom_run(app, argc, argv);
        }
    }
}

std::vector<CompletionItem> Console::auto_complete(Application &app, const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion)
{
    completion.clear();
    TokenParser parser;
    parser.parse(line, len, cursor);
    const auto &cursor_info = parser.get_cursor_info();

    // 1. if cursor is at the first token: 
    //      a. match sub-commands
    //      b. if (a) failed, try custom_complete()
    if (cursor_info.token_index == 0) {
        // to complete the first token with sub commands
        std::wstring temp_completion;
        auto commands = command_manager().match_by_prefix(cursor_info.prefix, temp_completion);
        if (!commands.empty()) {
            completion = temp_completion;
            std::vector<CompletionItem> result;
            result.reserve(commands.size());
            for (auto c : commands) {
                result.push_back(CompletionItem(c->name(), true));
            }
            return result;
        }
        else { // cannot match any sub-commands, try custom_complete()
            return custom_complete(app, line, len, cursor, completion);
        }
    }
    else { // 2. if the cursor is after the first token, and the first token is a sub command
        const Token &token0 = parser.tokens()[0];
        Command *cmd = command_manager().find_command(token0.value());
        if (cmd) {
            // pass sub arguments to the sub command
            const wchar_t *subline = token0.original_end() + 1;
            size_t sublen = (line + len) - (token0.original_end() + 1);
            return cmd->auto_complete(app, subline, sublen, cursor, completion);
        }
    }

    // 3. fallback to custom_complete()
    return custom_complete(app, line, len, cursor, completion);
}

std::vector<CompletionItem> Console::custom_complete(Application &, const wchar_t * /*line*/, size_t /*len*/, const wchar_t * /*cursor*/, std::wstring & completion)
{
    completion.clear();
    return std::vector<CompletionItem>();
}

void Console::custom_run(Application &, int argc, const wchar_t **argv)
{
    // the default implementation cannot handle any args
    fprintf(stderr, "Unknown command: \"");
    for (int i = 0; i < argc; i++) {
        if (i > 0)
            fprintf(stderr, " ");
        fprintf(stderr, "%ls", argv[i]);
    }
    fprintf(stderr, "\"\n");
}

void Console::on_enter_console(Application &app)
{
    show_help(app);
}

void Console::on_leave_console(Application &)
{
}

void Console::set_repeat_on_empty(bool enabled)
{
    repeat_on_empty_ = enabled;
}

void Console::show_help(Application & /*app*/)
{
    if (!command_manager().get_commands().empty()) {
        printf("Commands:\n");
        for (size_t i = 0; i < command_manager().get_commands().size(); i++) {
            Command *command = command_manager().get_commands()[i];
            if (!command->usage().empty())
                printf("  %ls", command->usage().c_str());
            else
                printf("  %ls", command->name().c_str());
            printf("\n");
        }
    }
    printf("  <Ctrl-D>: quit\n");
}

bool Console::show_command_help(Application & /*app*/, const std::wstring &command)
{
    Command *cmd = command_manager().find_command(command);
    if (cmd == nullptr)
        return false;
    if (!cmd->usage().empty())
        printf("  %ls", cmd->usage().c_str());
    else
        printf("  %ls", cmd->name().c_str());
    printf("\n");
    return true;
}

std::wstring Console::get_prompt(Application &)
{
    return name();
}

} // namespace exole
