#include "application.h"
#include "token_parser.h"
#include "file_name_completer.h"
#include "console.h"
#include "help_command.h"
#include "wcs_util.h"
#include <cstdlib>

using namespace exole;

const char HISTORY_FILE[]=".example_help_history";

class ConstantCommand: public Command
{
public:
    ConstantCommand(const wchar_t *name, const wchar_t *desc, const wchar_t *value)
    : Command(name)
    , value_(value)
    {
        std::wstring usage = name;
        usage += L": ";
        usage += desc;
        set_usage(usage);
    }
    void run(Application &app, int argc, const wchar_t **argv) override
    {
        printf("%ls\n", value_.c_str());
    }

private:
    std::wstring value_;
};

class ConstantConsole: public Console
{
public:
    ConstantConsole()
    : Console(L"const")
    {
        set_usage(L"const: show mathematical constants");

        // ref: https://en.wikipedia.org/wiki/Mathematical_constant
        command_manager().add_command(new ConstantCommand(L"pi", L"Archimedes' constant π", L"3.14159 26535 89793 23846 26433 83279 50288"));
        command_manager().add_command(new ConstantCommand(L"e", L"Euler's number e", L"2.71828 18284 59045 23536 02874 71352 66249"));
        command_manager().add_command(new ConstantCommand(L"sqrt2", L"square root of 2", L"1.41421 35623 73095 04880 16887 24209 69807"));
    }
    void on_enter_console(Application &app) override
    {
        show_help(app);
    }
};

int main(int argc, char *argv[])
{
    Application app;
    app.init(argv[0], nullptr, HISTORY_FILE);
    app.command_manager().add_command(new ConstantConsole);
    app.command_manager().add_command(new HelpCommand);
    app.run();
    return 0;
}
