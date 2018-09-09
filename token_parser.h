#ifndef EXOLE_TOKEN_PARSER_H
#define EXOLE_TOKEN_PARSER_H

#include <string>
#include <vector>

namespace exole {

enum TokenState
{
    TS_NORMAL,
    TS_DQUOTE,
    TS_SQUOTE,
    TS_ESCAPE,
    TS_QESCAPE,     // escape in double quotes
    TS_COMPLETE,
};

struct Token
{
private:
    // pointer to original text
    const wchar_t *begin_;
    const wchar_t *end_;

    // escaped text e.g. "a""\\b" -> a\b
    std::wstring value_;

    int cursor_; // default -1. Use cursor with value_ rather than begin_.
    TokenState state_;

public:
    Token();
    TokenState push(const wchar_t *pc, bool is_cursor);
    size_t original_size() const { return end_ - begin_; }
    const std::wstring &value() const { return value_; }
    const wchar_t *original_begin() const { return begin_; }
    const wchar_t *original_end() const { return end_; }
    TokenState state() const { return state_; }
    int cursor() const { return cursor_; }
    void check_cursor(const wchar_t *cursor);
};

struct CursorInfo
{
    size_t token_index;
    std::wstring prefix;

    CursorInfo()
    : token_index(0)
    {
    }
};

class TokenParser
{
public:
    void parse(const wchar_t *line, size_t len, const wchar_t *cursor);

    const std::vector<Token> &tokens() const { return tokens_; }
    const CursorInfo &get_cursor_info() const { return cursor_info_; }
private:
    /// @return -1 if not found
    int find_token_at_cursor() const;

    std::vector<Token> tokens_;
    CursorInfo cursor_info_;
};

} // namespace exole

#endif // EXOLE_TOKEN_PARSER_H
