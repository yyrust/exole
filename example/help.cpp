#include "application.h"
#include "token_parser.h"
#include "file_name_completer.h"
#include "console.h"
#include "help_command.h"
#include "wcs_util.h"
#include "constant_console.h"
#include <cstdlib>

using namespace exole;

const char HISTORY_FILE[]=".example_help_history";

int main(int argc, char *argv[])
{
    Application app;
    app.init(argv[0], nullptr, HISTORY_FILE);
    app.command_manager().add_command(new ConstantConsole);
    app.command_manager().add_command(new HelpCommand);
    app.run();
    return 0;
}
