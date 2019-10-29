#include <clocale>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <histedit.h>
#include "util.h"
#include "application.h"
#include "console.h"
#include "token_parser.h"
#include "wcs_util.h"
#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>

namespace exole {

class RootConsole : public Console
{
public:
    RootConsole()
    : Console(L"")
    {}

    void set_prompt(const std::wstring &prompt)
    {
        prompt_ = prompt;
    }
    std::wstring get_prompt(Application &) override
    {
        return prompt_;
    }
private:
    std::wstring prompt_;
};

class EditlineWrapper
{
public:
    EditlineWrapper()
    : history_(nullptr)
    , editline_(nullptr)
    {}
    HistoryW *history_;
    EditLine *editline_;
};

// NOTE: function signature : el_func_t (declared in libedit/src/map.h)
static el_action_t complete_handler(EditLine *editline, wint_t ch);
// NOTE: function signature : el_pfunc_t (declared in libedit/src/map.h)
static wchar_t *prompt_handler(EditLine *editline);

Application::Application()
: root_(new RootConsole())
, prompt_(L"> ")
, is_batch_mode_(false)
{
    el_.reset(new EditlineWrapper);
    console_stack_.push_back(root_.get());
}

Application::~Application()
{
    if (el_->editline_) {
        el_end(el_->editline_);
    }
    if (el_->history_) {
        HistEventW event;
        history_w(el_->history_, &event, H_SAVE, history_file_.c_str()); // save history
        history_wend(el_->history_);
    }
}

CommandManager &Application::command_manager()
{
    return root_->command_manager();
}

// for reference: https://github.com/seanchann/libcutil (libcutil/src/core/core.c, function cli_complete() )
el_action_t complete_handler(EditLine *editline, wint_t /*ch*/)
{
    Application *self = nullptr;
    el_wget(editline, EL_CLIENTDATA, &self);

    const LineInfoW *line_info = el_wline(editline);

    size_t buffer_len = line_info->lastchar - line_info->buffer;
    std::wstring completion;
    auto candidates = self->current_console()->auto_complete(*self, line_info->buffer, buffer_len, line_info->cursor, completion);
    if (candidates.empty()) {
        return CC_ERROR;
    }
    else if (candidates.size() == 1) {
        if (candidates[0].is_complete()) {
            completion += L' ';
        }
        el_winsertstr(editline, completion.c_str());
        return CC_REDISPLAY;
    }
    else if (candidates.size() > 1) {
        if (completion.empty()) { // cannot complete any more, just show candidates
            printf("\n");
            int line_length = 0;
            for (auto candidate : candidates) {
                const int MAX_LINE_LENGTH = 80;
                const int TAB_WIDTH = 2;
                // output spaces
                if (line_length != 0) {
                    int num_spaces = TAB_WIDTH - line_length % TAB_WIDTH;
                    for (int i = 0; i < num_spaces; i++) {
                        printf(" ");
                    }
                    line_length += num_spaces;
                }
                if (line_length + candidate.value().size() > MAX_LINE_LENGTH) {
                    printf("\n");
                    line_length = 0;
                }

                // print a candidate
                line_length += printf("%ls", candidate.value().c_str());
            }
            printf("\n");
            return CC_REDISPLAY;
        }
        else {
            el_winsertstr(editline, completion.c_str());
            return CC_REDISPLAY;
        }
    }
    return CC_NORM;
}

wchar_t *prompt_handler(EditLine *editline)
{
    Application *self = nullptr;
    el_wget(editline, EL_CLIENTDATA, &self);
    return const_cast<wchar_t *>(self->get_prompt().c_str());
}

void Application::init(const char *prog_name, std::unique_ptr<CommandContext> context, const std::string &history_file)
{
    history_file_ = history_file;
    context_ = std::move(context);

    setlocale(LC_ALL, "");

    HistoryW *history = history_winit();
    HistEventW event;
    history_w(history, &event, H_SETSIZE, 1000); // remember 1000 events
    history_w(history, &event, H_LOAD, history_file_.c_str()); // load histroy
    history_w(history, &event, H_SETUNIQUE, 1); // adjacent identical event strings should not be entered into the history

    EditLine *editline = el_init(prog_name, stdin, stdout, stderr);
    el_wset(editline, EL_SIGNAL, 1); // handle signals gracefully

    // NOTE: editline 的api分为宽字符版(例如el_wset/el_wget/...)和ascii字符版(例如el_set/el_get/...)。
    //       对于前者，传入的字符串参数必须是宽字符串，否则无法被正确解析。

    el_wset(editline, EL_CLIENTDATA, this);
    el_wset(editline, EL_EDITOR, L"emacs"); // use emacs style key bindings
    el_wset(editline, EL_HIST, history_w, history);
    el_wset(editline, EL_PROMPT, prompt_handler);

    // Register function complete_handler() as command "ed-complete"
    el_wset(editline, EL_ADDFN, L"ed-complete", L"Complete argument", complete_handler);
    /* Bind <tab> to command "ed-complete" */
    el_wset(editline, EL_BIND, L"^I", L"ed-complete", NULL);
    // for reference:  https://github.com/seanchann/libcutil (libcutil/src/core/elhelper.c)

    // Bind ctrl-r to builtin command em-inc-search-prev
    el_wset(editline, EL_BIND, L"^R", L"em-inc-search-prev", NULL);

    // Let ctrl-w delete just the previous word, otherwise it will delete to the beginning.
    el_wset(editline, EL_BIND, L"^W", L"ed-delete-prev-word", NULL);

    // NOTE: The following line will show all key-bindings, useful for debugging.
    //el_wset(editline, EL_BIND, NULL);

    el_->history_ = history;
    el_->editline_ = editline;
}

void Application::run()
{
    current_console()->on_enter_console(*this);
    while (true) {
        const wchar_t *line = NULL;
        int num = 0;
        line = el_wgets(el_->editline_, &num);
        if (line == NULL || num == 0) {
            leave_console();
            if (console_stack_.empty()) {
                break;
            }
            else {
                printf("\n");
                current_console()->on_enter_console(*this);
                continue;
            }
        }

        if (wcslen(line) != 0 && 0 != wcscmp(L"\n", line)) {
            HistEventW event;
            history_w(el_->history_, &event, H_ENTER, line);
        }

        TokenizerW *tok = tok_winit(NULL);
        EXOLE_SCOPE_EXIT(tok, [](TokenizerW *t) { tok_wend(t); });

        int argc;
        const wchar_t **argv;
        int ret = tok_wstr(tok, line, &argc, &argv);
        if (ret < 0) { // internal error
            fprintf(stderr, "failed to parse input (internal error)\n");
            continue;
        }
        else if (ret > 0) { // need to read more lines until quotes are matched
            // TODO
        }
        else { // ret == 0, successful
            current_console()->run(*this, argc, argv);
        }
    }
}

void Application::init_batch_mode(const char * /*prog_name*/, std::unique_ptr<CommandContext> context)
{
    is_batch_mode_ = true;

    context_ = std::move(context);
    setlocale(LC_ALL, "");

    current_console()->on_enter_console(*this);
}

void Application::run_command(const std::wstring &line)
{
    if (line.empty()) {
        return;
    }

    TokenizerW *tok = tok_winit(NULL);
    EXOLE_SCOPE_EXIT(tok, [](TokenizerW *t) { tok_wend(t); });

    int argc;
    const wchar_t **argv;
    int ret = tok_wstr(tok, line.c_str(), &argc, &argv);

    // partial input is not supported in batch mode
    switch (ret) {
    case 0: // successful
        break;
    case -1: // internal error
        fprintf(stderr, "failed to parse command: internal error\n");
        break;
    case 1: // unmatched single quote
        fprintf(stderr, "failed to parse command: unmatched single quote\n");
        break;
    case 2: // unmatched double quote
        fprintf(stderr, "failed to parse command: unmatched double quote\n");
        break;
    case 3: // backslash quoted
        fprintf(stderr, "failed to parse command: backslash quoted\n");
        break;
    default: // undocumented error
        fprintf(stderr, "failed to parse command: unknown error %d\n", ret);
        break;
    }
    if (ret != 0)
        return;

    current_console()->run(*this, argc, argv);
}

void Application::enter_console(Console *console)
{
    console_stack_.push_back(console);
    update_prompt();
}

void Application::leave_console()
{
    console_stack_.back()->on_leave_console(*this);
    console_stack_.pop_back();
    update_prompt();
}

void Application::update_prompt()
{
    prompt_.clear();
    for (size_t i = 0; i < console_stack_.size(); i++) {
        Console *console = console_stack_[i];
        std::wstring name = console->get_prompt(*this);
        if (!prompt_.empty()) {
            prompt_ += L'/';
        }
        prompt_ += name;
    }
    prompt_ += L"> ";
}

void Application::set_default_prompt(const std::wstring &prompt)
{
    root_->set_prompt(prompt);
    update_prompt();
}

bool Application::get_window_size(unsigned *rows, unsigned *cols)
{
    if (!rows || !cols) {
        return false;
    }
#ifdef TIOCGSIZE
	struct ttysize ts;
	int err = ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	*cols = ts.ts_cols;
	*rows = ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	int err = ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	*cols = ts.ws_col;
	*rows = ts.ws_row;
#endif /* TIOCGSIZE */

// NOTE: https://stackoverflow.com/a/50769952 explains the difference between TIOCGWINSZ and TIOCGSIZE.

    return err != -1;
}

int Application::getc(wchar_t *ch)
{
    if (is_batch_mode_) // editline is N/A
        return -1;
    else
        return el_wgetc(el_->editline_, ch);
}

} // namespace exole
