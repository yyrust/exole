#ifndef EXOLE_ARGUMENTS_H
#define EXOLE_ARGUMENTS_H

namespace exole {
namespace detail {

class Arguments {
public:
    int argc() const { return argc_; }
    const wchar_t **argv() const { return (const wchar_t **)argv_; }

    void set(int argc, const wchar_t **argv);

    void freeze() { frozen_ = true; }
    void unfreeze() { frozen_ = false; }

    Arguments();
    ~Arguments();
private:
    void clear();
    int argc_;
    wchar_t **argv_;
    bool frozen_;
};

} // namespace detail
} // namespace exole

#endif // EXOLE_ARGUMENTS_H
