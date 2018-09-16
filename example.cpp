#include "application.h"
#include "token_parser.h"
#include "file_name_completer.h"
#include "console.h"
#include "wcs_util.h"
#include <cstdlib>

using namespace exole;

const char HISTORY_FILE[]=".econsole_history";

class FileViewContext : public CommandContext
{
public:
    bool set_file(const std::string &file_path)
    {
        file_path_ = file_path;
        if (fp_)
            fclose(fp_);
        fp_ = fopen(file_path_.c_str(), "r");
        if (!fp_) {
            fprintf(stderr, "ERROR: cannot open file %s\n", file_path.c_str());
            return false;
        }
        fseek(fp_, 0, SEEK_END);
        length_ = ftell(fp_);
        rewind(fp_);
        return true;
    }

    void rewind_file()
    {
        if (fp_)
            rewind(fp_);
    }

    std::string file_path_;
    FILE *fp_;
    size_t length_;

    FileViewContext()
    : fp_(nullptr)
    , length_(0)
    {}

    ~FileViewContext()
    {
        if (fp_)
            fclose(fp_);
    }
};

class FileCommand : public Command
{
public:
    FileCommand()
    : Command(L"file")
    {
        set_usage(L"file: open a file");
    }

    void run(Application &app, int argc, const wchar_t **argv) override
    {
        if (argc == 1) {
            FileViewContext *context = dynamic_cast<FileViewContext *>(app.context());
            if (!context) {
                printf("invalid context\n");
                return;
            }
            if (context->set_file(wcs_to_mbs(argv[0]).c_str()))
                printf("file selected: %ls, size: %zu\n", argv[0], context->length_);

            // set file name as console prompt
            app.set_default_prompt(L'[' + mbs_to_wcs(basename(context->file_path_)) + L']');
        }
    }

    std::vector<CompletionItem> auto_complete(Application &, const wchar_t *line, size_t len, const wchar_t *cursor, std::wstring &completion) override
    {
        std::vector<CompletionItem> result;
        TokenParser parser;
        parser.parse(line, len, cursor);
        if (parser.tokens().empty()) {
            return transform(FileNameCompleter::complete(nullptr, 0, completion, FT_ALL_TYPES));
        }
        else if (parser.tokens().size() == 1) {
            const Token &token = parser.tokens()[0];
            if (token.cursor() < 0)
                return result;
            return transform(FileNameCompleter::complete(token.value().c_str(), token.cursor(), completion, FT_ALL_TYPES));
        }
        else {
            return result;
        }
    }

    static std::vector<CompletionItem> transform(const std::vector<FileNameCompletionItem> &items)
    {
        std::vector<CompletionItem> result;
        result.reserve(items.size());
        for (const auto &item : items) {
            result.push_back(CompletionItem(item.value(), item.type() != FT_DIRECTORY));
        }

        return result;
    }

    static std::string basename(const std::string &file_path)
    {
        size_t sep = file_path.find_last_of("/\\");
        if (sep == file_path.npos)
            return file_path;
        else
            return file_path.substr(sep + 1);
    }
};

class NextHexCommand: public Command
{
public:
    static const size_t LINE_LENGTH = 16;
    NextHexCommand()
    : Command(L"next")
    {
        set_usage(L"next [N]: show next N lines of hex data");
    }
    void run(Application &app, int argc, const wchar_t **argv) override
    {
        FileViewContext *context = dynamic_cast<FileViewContext *>(app.context());
        if (!context) {
            return;
        }
        if (!context->fp_) {
            fprintf(stderr, "ERROR: file is not ready\n");
            return;
        }

        // parse row count
        const int DEFAULT_ROWS = 10;
        int rows = DEFAULT_ROWS;
        if (argc >= 1) {
            rows = atoi(wcs_to_mbs(argv[0]).c_str());
            if (rows <= 0) {
                fprintf(stderr, "ERROR: invalid line number: '%ls'\n", argv[0]);
                return;
            }
        }

        for (int r = 0; r < rows; r++) {
            unsigned char buf[LINE_LENGTH];
            size_t n = fread(buf, 1, LINE_LENGTH, context->fp_);
            if (n == 0)
                break;
            uint64_t offset = ftell(context->fp_);

            print_line(offset, buf, n);

            if (n < LINE_LENGTH) { // no more lines
                break;
            }
        }

        // print progress
        uint64_t offset = ftell(context->fp_);
        printf("-------- %llu%% -------- ", 100ull * offset / context->length_);
        if (offset < context->length_)
            printf("press ENTER to continue --------\n");
        else
            printf("finished --------\n");
    }

    void print_line(uint64_t file_offset, const unsigned char *data, size_t length) const
    {
        // print file offset
        printf("[%08llx]", (unsigned long long)file_offset);

        // print hex data
        for (size_t i = 0; i < LINE_LENGTH; i++) {
            if (i % 4 == 0)
                printf(" ");
            if (i < length)
                printf(" %02x", data[i]);
            else
                printf("   ");
        }

        printf("  -  ");

        // print as text
        for (size_t i = 0; i < length; i++) {
            if (data[i] >= 32 && data[i] <127 && isprint(data[i]))
                printf("%c", data[i]);
            else
                printf(".");
        }

        printf("\n");
    }
};

class HexView: public Console
{
public:
    HexView()
    : Console(L"hex")
    {
        set_usage(L"hex:  view hex data");
        command_manager().add_command(new NextHexCommand);
    }
    void on_enter_console(Application &app) override
    {
        FileViewContext *context = dynamic_cast<FileViewContext *>(app.context());
        context->rewind_file();
        show_help(app);
    }
};

int main(int argc, char *argv[])
{
    Application app;
    auto context = std::make_unique<FileViewContext>();
    app.init(argv[0], std::move(context), HISTORY_FILE);
    app.command_manager().add_command(new FileCommand);
    app.command_manager().add_command(new HexView);
    app.run();
    return 0;
}
