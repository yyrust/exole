#ifndef EXOLE_COMMAND_MANAGER_H
#define EXOLE_COMMAND_MANAGER_H

#include "command.h"
#include <map>
#include <vector>

namespace exole {

class CommandManager
{
public:
    typedef std::map<std::wstring, Command *> CommandMap;
    typedef std::vector<Command *> CommandVector;

    ~CommandManager();

    bool add_command(Command *command);

    Command *find_command(const std::wstring &name);

    std::vector<Command *> match_by_prefix(const std::wstring &prefix, std::wstring &completion) const;

    const CommandVector &get_commands() const { return commands_; }

private:
    CommandMap command_map_;
    CommandVector commands_;
};

} // namespace exole

#endif // EXOLE_COMMAND_MANAGER_H
