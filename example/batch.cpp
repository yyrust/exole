#include "application.h"
#include "token_parser.h"
#include "file_name_completer.h"
#include "console.h"
#include "help_command.h"
#include "wcs_util.h"
#include "batch_mode_args.h"
#include "constant_console.h"
#include <cstdlib>

using namespace exole;

const char HISTORY_FILE[]=".example_batch_history";

int main(int argc, char *argv[])
{
    Application app;

    // 1. Handle batch mode arguments.
    // These arguments will be removed from argv after parsing, and argc will be updated accordingly.
    //
    // Batch mode options:
    //      -b: enable batch mode
    //      -x <command>: execute <command>
    //      -f <file>: read commands from <file> and execute them
    //
    // Example usage:
    //      ./example_batch -b -x 'const pi' -x const -x pi -f commands.txt -f more_commands.txt
    BatchModeArgs args;

    // These options are not mandatory. Change the name to '\0' for unnecessary options.
    const char BATCH_OPT_NAME = 'b';
    const char EXEC_OPT_NAME = 'x';
    const char FILE_OPT_NAME = 'f';

    bool ok = args.parse(argc, argv, BATCH_OPT_NAME, EXEC_OPT_NAME, FILE_OPT_NAME);
    if (!ok) {
        return -1;
    }
    ok = args.load_commands();
    if (!ok) {
        return -1;
    }

    // 2. Handle other arguments. In this example we simply print them.
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] %s\n", i, argv[i]);
    }

    // 3. Initialize the application.
    if (args.batch_mode_enabled()) {
        // Use init_batch_mode() for batch mode.
        app.init_batch_mode(argv[0], nullptr);
    }
    else {
        // Use init() for interactive mode.
        app.init(argv[0], nullptr, HISTORY_FILE);
    }

    app.command_manager().add_command(new ConstantConsole);
    app.command_manager().add_command(new HelpCommand);

    // 4. Run the application.
    if (args.batch_mode_enabled()) {
        // In batch mode, run commands one by one.

        // run commands passed via '-x'
        for (auto c: args.commands()) {
            auto command = mbs_to_wcs(c, strlen(c));
            app.run_command(command);
        }

        // run commands loaded from files passed via '-f'
        for (auto c: args.file_commands()) {
            auto command = mbs_to_wcs(c);
            app.run_command(command);
        }
    }
    else {
        // In interactive mode, just enter the normal run loop.
        app.run();
    }
    return 0;
}
