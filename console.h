#ifndef EXOLE_CONSOLE_H
#define EXOLE_CONSOLE_H

#include "command.h"
#include "command_context.h"
#include "command_manager.h"

namespace exole {

class Application;

namespace detail { class Arguments; }

class Console : public Command
{
public:
    Console(std::wstring name);
    ~Console();
    CommandManager &command_manager() { return command_manager_; }
    void run(Application &app, int argc, const wchar_t **argv) override;

    std::vector<CompletionItem> auto_complete(Application &app, const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion) override;

    virtual std::wstring get_prompt(Application &app);

    // If the input is empty, repeat the last command. Enabled by default.
    void set_repeat_on_empty(bool enabled);
    void show_help(Application &app);
    /// You can print some hints when the user enters this console.
    virtual void on_enter_console(Application &app);
    virtual void on_leave_console(Application &app);
protected:
    virtual std::vector<CompletionItem> custom_complete(Application &app, const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion);

    virtual void custom_run(Application &app, int argc, const wchar_t **argv);
private:
    CommandManager command_manager_;
    detail::Arguments *last_arguments_;
    bool repeat_on_empty_;
};

} // namespace exole

#endif // EXOLE_CONSOLE_H
