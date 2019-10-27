#include "batch_mode_args.h"
#include <fstream>
#include <cassert>
#include <cstring>

namespace exole {

BatchModeArgs::BatchModeArgs()
: batch_mode_enabled_(false)
{
}

bool BatchModeArgs::parse(int &argc, char **argv, char batch_option, char exec_option, char file_option)
{
    int fast = 0, slow = 0;

    enum State {
        EXPECT_MAYBE_OPTION,
        EXPECT_ARGUMENT,
        EXPECT_LAST_ARGUMENT,
    } state = EXPECT_MAYBE_OPTION;

    enum ArgType {
        ARGUMENT,
        OPTION_0,       // requires no argument
        OPTION_1,       // requires one argument
        OTHER_OPTION,   // options we don't care
        DOUBLE_DASH,    // marks end of options
    };

    bool stop = false;
    size_t num_errors = 0;

    char last_opt = '\0'; // the option waiting for an argument
    while (fast < argc && !stop) {
        char *arg = argv[fast];
        size_t len = strlen(arg);

        // determine arg type
        ArgType type;
        char opt = '\0';
        if (state == EXPECT_LAST_ARGUMENT) { // after "--"
            type = ARGUMENT;
        }
        else if (len < 2 || arg[0] != '-') {
            type = ARGUMENT;
        }
        else { // arg len >= 2 and arg starts with '-'
            opt = arg[1];
            if (len == 2 && '-' == opt) {
                type = DOUBLE_DASH;
            }
            else if (batch_option == opt) {
                if (len > 2) { // e.g. "-byes"
                    fprintf(stderr, "error: invalid option %s: -%c does not accept arguments\n", arg, batch_option);
                    num_errors++;
                }
                else {
                    batch_mode_enabled_ = true;
                }
                type = OPTION_0;
            }
            else if (exec_option == opt || file_option == opt) {
                type = OPTION_1;
                if (len > 2) { // e.g. "-xcommand" or "-f/tmp/commands.txt"
                    type = OPTION_0;
                    const char *opt_arg = arg + 2;
                    if (exec_option == opt)
                        commands_.push_back(opt_arg);
                    else
                        files_.push_back(opt_arg);
                }
            }
            else {
                type = OTHER_OPTION;
            }
        }

        // handle the option/argument
        bool to_consume = false;
        switch (state) {
        case EXPECT_MAYBE_OPTION:
            switch (type) {
            case OPTION_0:
                to_consume = true;
                // fallthrough
            case OTHER_OPTION:
            case ARGUMENT:
                // keep current state
                break;
            case OPTION_1:
                last_opt = opt;
                state = EXPECT_ARGUMENT;
                to_consume = true;
                break;
            case DOUBLE_DASH:
                stop = true;
                break;
            }
            break;
        case EXPECT_LAST_ARGUMENT:
            stop = true;
            assert(type == ARGUMENT);
            // fallthrough
        case EXPECT_ARGUMENT:
            switch (type) {
            case OPTION_0:
                fprintf(stderr, "error: option -%c requires an argument\n", last_opt);
                num_errors++;
                state = EXPECT_MAYBE_OPTION;
                to_consume = true;
                break;
            case OPTION_1:
                fprintf(stderr, "error: option -%c requires an argument\n", last_opt);
                num_errors++;
                state = EXPECT_ARGUMENT;
                last_opt = opt;
                to_consume = true;
                break;
            case OTHER_OPTION:
                fprintf(stderr, "error: option -%c requires an argument\n", last_opt);
                num_errors++;
                state = EXPECT_MAYBE_OPTION;
                break;
            case ARGUMENT:
                state = EXPECT_MAYBE_OPTION;
                if (last_opt == exec_option) {
                    commands_.push_back(arg);
                }
                else {
                    files_.push_back(arg);
                }
                to_consume = true;
                break;
            case DOUBLE_DASH:
                state = EXPECT_LAST_ARGUMENT;
                break;
            }
            break;
        } // switch (state)

        // move unconsumed args to slow
        if (! to_consume) {
            if (slow < fast) {
                argv[slow] = argv[fast];
            }
            slow++;
        }
        fast++;

    } // while

    if (state != EXPECT_MAYBE_OPTION) {
        fprintf(stderr, "error: option -%c requires an argument\n", last_opt);
        num_errors++;
    }

    // move unconsumed args to slow
    if (slow < fast) {
        for (; fast < argc; fast++, slow++) {
            argv[slow] = argv[fast];
        }
    }

    argc = slow;
    return num_errors == 0;
}

static bool is_empty_or_comment(const std::string &line)
{
    size_t pos = line.find_first_not_of(" \t");
    if (pos == line.npos) { // is empty line
        return true;
    }
    if (line[pos] == '#') { // is comment
        return true;
        // XXX: what if somebody wants a command starting with '#'?
    }
    return false;
}

bool BatchModeArgs::load_commands(const char *filename, std::deque<std::string> &commands)
{
    std::ifstream ifs(filename);
    if (!ifs) {
        fprintf(stderr, "cannot open file '%s'\n", filename);
        return false;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        if (is_empty_or_comment(line))
            continue;
        commands.push_back(line);
    }
    return true;
}

bool BatchModeArgs::load_commands()
{
    for (auto fn: files_) {
        if (!load_commands(fn, file_commands_)) {
            return false;
        }
    }
    return true;
}

} // namespace exole
