#include "token_parser.h"
#include <cassert>
#include <cwctype>

namespace exole {

Token::Token()
    : begin_(nullptr)
    , end_(nullptr)
    , cursor_(-1)
    , state_(TS_NORMAL)
{
}

TokenState Token::push(const wchar_t *pc, bool is_cursor)
{
    if (is_cursor) {
        cursor_ = value_.size();
    }
    if (!begin_) {
        begin_ = pc;
    }
    /* state chart:
    NORMAL: SPACE->COMPLETE, '->SQUOTE, "->DQUOTE, \->ESCAPE, OTHER->NORMAL
    DQUOTE: "->NORMAL, \->QESCAPE, OTHER->DQUOTE
    SQUOTE: '->NORMAL, OTHER->SQUOTE
    ESCAPE: OTHER->NORMAL
    QESCAPE: OTHER->DQUOTE
    */
    const wchar_t c = *pc;
    switch (state_) {
    case TS_COMPLETE:
        return TS_COMPLETE;
        break;
    case TS_NORMAL:
        // NORMAL: SPACE->COMPLETE, '->SQUOTE, "->DQUOTE, \->ESCAPE, OTHER->NORMAL
        if (iswspace(c)) {
            end_ = pc;
            state_ = TS_COMPLETE;
            return TS_COMPLETE;
        }
        switch (c) {
        case L'\\':
            state_ = TS_ESCAPE;
            break;
        case L'"':
            state_ = TS_DQUOTE;
            break;
        case L'\'':
            state_ = TS_SQUOTE;
            break;
        default:
            value_.push_back(c);
            break;
        }
        break;
    case TS_DQUOTE:
        // DQUOTE: "->NORMAL, \->QESCAPE, OTHER->DQUOTE
        switch (c) {
        case L'"':
            state_ = TS_NORMAL;
            break;
        case L'\\':
            state_ = TS_QESCAPE;
            break;
        default:
            value_.push_back(c);
            break;
        }
        break;
    case TS_SQUOTE:
        // SQUOTE: '->NORMAL, OTHER->SQUOTE
        switch (c) {
        case L'\'':
            state_ = TS_NORMAL;
            break;
        default:
            value_.push_back(c);
            break;
        }
        break;
    case TS_ESCAPE:
        // ESCAPE: OTHER->NORMAL
        value_.push_back(c);
        state_ = TS_NORMAL;
        break;
    case TS_QESCAPE:
        // QESCAPE: OTHER->DQUOTE
        value_.push_back(c);
        state_ = TS_DQUOTE;
        break;
    }
    end_ = pc + 1;
    return state_;
}

void Token::check_cursor(const wchar_t *cursor)
{
    if (cursor == end_) {
        cursor_ = value_.size();
    }
}

static const wchar_t *skip_space(const wchar_t *p, const wchar_t *end)
{
    while (p < end && iswspace(*p))
        p++;
    return p;
}

void TokenParser::parse(const wchar_t *line, size_t len, const wchar_t *cursor)
{
    tokens_.clear();
    const wchar_t *end = line + len;
    const wchar_t *p = skip_space(line, end);
    while (p < end) {
        Token token;
        TokenState ts;
        do {
            ts = token.push(p, p == cursor);
            p++;
        } while (ts != TS_COMPLETE && p < end);
        assert(token.original_size() > 0);
        tokens_.push_back(token);
        p = skip_space(p, end);
    }
    if (cursor == end) {
        // in this case, the cursor cannot be pushed to the last token, we should call Token::check_cursor() to correct it.
        if (!tokens_.empty()) {
            tokens_.back().check_cursor(cursor);
        }
    }

    // update cursor info
    int cursor_index = find_token_at_cursor();
    if (cursor_index == -1) {
        cursor_info_.prefix.clear();
        if (tokens_.empty()) {
            cursor_info_.token_index = 0;
        }
        else if (cursor < tokens_[0].original_begin()) {
            cursor_info_.token_index = 0;
        }
        else {
            assert(cursor > tokens_.back().original_end());
            cursor_info_.token_index = tokens_.size();
        }
    }
    else {
        const Token &t = tokens_[cursor_index];
        cursor_info_.token_index = cursor_index;
        cursor_info_.prefix = t.value().substr(0, t.cursor());
    }
}

int TokenParser::find_token_at_cursor() const
{
    for (int i = 0; i < (int)tokens_.size(); i++) {
        if (tokens_[i].cursor() >= 0)
            return i;
    }
    return -1;
}

} // namespace exole
