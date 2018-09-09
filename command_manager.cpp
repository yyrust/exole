#include "command_manager.h"
#include "wcs_util.h"
#include <cstring>
#include <cassert>
#include <limits>

namespace exole {

CommandManager::~CommandManager()
{
    for (auto c : commands_) {
        delete c;
    }
}

bool CommandManager::add_command(Command *command)
{
    auto result = command_map_.insert(std::make_pair(command->name(), command));
    if (!result.second) {
        // TODO: log
        return false;
    }
    commands_.push_back(command);
    return true;
}

Command *CommandManager::find_command(const std::wstring &name)
{
    auto it = command_map_.find(name);
    return it == command_map_.end() ? nullptr : it->second;
}

std::vector<Command *> CommandManager::match_by_prefix(const std::wstring &prefix, std::wstring &completion) const
{
    return exole::match_by_prefix(commands_, [](Command *cmd) { return cmd->name(); }, prefix, completion);
}

} // namespace exole
