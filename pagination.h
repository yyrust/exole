#ifndef EXOLE_PAGINATION_H
#define EXOLE_PAGINATION_H

#include "application.h"

namespace exole {

class Pagination
{
public:
    Pagination();

    /// Should be called before printing each page.
    void start_page(Application &app);

    /// Should be called after printing each page, unless there is nothing more to print.
    /// \retval true continue printing
    /// \retval false quit printing
    bool next_page(Application &app);

    /// \return the row count of the terminal window.
    /// \note Each call to start_page() updates this value.
    unsigned int rows() const;

    /// \return the column count of the terminal window.
    /// \note Each call to start_page() updates this value.
    unsigned int cols() const;

private:
    unsigned rows_, cols_;
};

} // namespace exole

#endif // EXOLE_PAGINATION_H
