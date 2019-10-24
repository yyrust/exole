#ifndef EXOLE_APPLICATION_H
#define EXOLE_APPLICATION_H

#include <histedit.h>
#include "command.h"
#include "command_manager.h"
#include "command_context.h"
#include <memory>

namespace exole {

typedef unsigned char el_action_t;

class Console;
class RootConsole;

class Application
{
public:
    Application();
    ~Application();
    void init(const char *prog_name, std::unique_ptr<CommandContext> context, const std::string &history_file);
    void run();

    void init_batch_mode(const char *prog_name, std::unique_ptr<CommandContext> context);
    void run_command(const std::wstring &line); // for batch mode only

    bool is_batch_mode() const { return is_batch_mode_; }

    CommandManager &command_manager();
    void enter_console(Console *console);
    void leave_console();
    Console *current_console() const { return console_stack_.back(); }
    CommandContext *context() { return context_.get(); }
    void set_default_prompt(const std::wstring &prompt);
    void update_prompt();

    /// Get the size of the terminal window.
    static bool get_window_size(unsigned *rows, unsigned *cols);
    /// Read a character from the tty.
    /// \return the number of characters read if successful, -1 otherwise.
    int getc(wchar_t *ch);
private:
    HistoryW *history_;
    EditLine *editline_;
    std::unique_ptr<RootConsole> root_;
    std::vector<Console *> console_stack_;
    std::unique_ptr<CommandContext> context_;
    std::wstring prompt_;
    std::string history_file_;
    bool is_batch_mode_;

    // NOTE: function signature : el_func_t (declared in libedit/src/map.h)
    static el_action_t complete_handler(EditLine *editline, wint_t ch);
    // NOTE: function signature : el_pfunc_t (declared in libedit/src/map.h)
    static wchar_t *prompt_handler(EditLine *editline);
};

} // namespace exole

#endif // EXOLE_APPLICATION_H
