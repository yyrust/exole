#ifndef EXOLE_BATCH_MODE_ARGS_H
#define EXOLE_BATCH_MODE_ARGS_H

#include <vector>
#include <deque>
#include <string>

namespace exole {

/**
 * BatchModeArgs can parse and remove batch mode options and arguments from normal command line arguments (argc, argv).
 * See usage in \c example/batch.cpp .
 *
 * This utility class is provided for convenience and testing, and has significant limitations:
 *
 * - Double-dash is required for long options.
 * - Short options cannot be combined into one like 'tar -xzfMYFILE.tar.gz', because BatchModeArgs has no knowledge
 *   about other options. For example, '-x' is the exec-option used by BatchModeArgs, and '-t' is an option unknown to
 *   BatchModeArgs, then "-txcommand" can be interpreted as either "-t -x command" or "-t xcommand", depending on
 *   whether option '-t' takes an argument.
 *
 * So if your program needs to parse other options, BatchModeArgs may not be a good choice for parser.
 */
class BatchModeArgs
{
public:
    BatchModeArgs();

    bool parse(int &argc, char **argv, char batch_option, char exec_option, char file_option);
    bool load_commands();

    bool batch_mode_enabled() const { return batch_mode_enabled_; }

    /// Commands passed via exec_option.
    const std::vector<const char *> &commands() const { return commands_; }

    /// Commands loaded from files specified via file_option.
    const std::deque<std::string> &file_commands() const { return file_commands_; }

    const std::vector<const char *> &files() const { return files_; }

    static bool load_commands(const char *filename, std::deque<std::string> &commands);

private:
    std::vector<const char *> commands_;
    std::vector<const char *> files_;
    std::deque<std::string> file_commands_;
    bool batch_mode_enabled_;
};

} // namespace exole

#endif // EXOLE_BATCH_MODE_ARGS_H
