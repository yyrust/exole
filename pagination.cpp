#include "pagination.h"

namespace exole {

Pagination::Pagination()
: rows_(0)
, cols_(0)
{
}

void Pagination::start_page(Application &app)
{
    app.get_window_size(&rows_, &cols_);
}

bool Pagination::next_page(Application &app)
{
    if (app.is_batch_mode()) {
        return true;
    }
    printf("(ENTER:continue / q:quit)");
    wchar_t ch = 0;
    enum { INVALID, QUIT, CONTINUE } choice = INVALID;
    const wchar_t KEY_q = 'q';
    const wchar_t KEY_Q = 'Q';
    const wchar_t KEY_ENTER = '\n';
    // choose to continue or to stop
    do {
        int num = app.getc(&ch);
        if (num == 1) {
            if (ch == KEY_q|| ch == KEY_Q) {
                choice = QUIT;
                printf("\n");
            }
            else if (ch == KEY_ENTER) {
                choice = CONTINUE;
                printf("\r");
            }
        }
    } while (choice == INVALID);
    return choice == CONTINUE;
}

unsigned int Pagination::rows() const { return rows_; }

unsigned int Pagination::cols() const { return cols_; }

} // namespace exole
