#ifndef _FLANER_LEXER_IO_HH_
#define _FLANER_LEXER_IO_HH_

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
//#include <filesystem>

namespace flaner
{
namespace lexer
{
namespace io
{
    enum class Encoding
    {
        UTF_8,
        UTF_16,
        UTF_32,
    };

    enum class OpenMode
    {
        Interactive,
        OpenExisting,
    };

    class Source
    {
    public:
        Source(std::string path, Encoding encoding = Encoding::UTF_8)
            : path(path),
            encoding(encoding),
            openMode(OpenMode::OpenExisting)
        {
            object.open(path, std::ios::in | std::ios::binary);
        }
        Source(const Source& s)
            : path(s.path),
            encoding(s.encoding),
            openMode(s.openMode)
        {

        }
        Source()
            : path(""),
            encoding(Encoding::UTF_8),
            openMode(OpenMode::Interactive)
        {
            std::wcin.rdbuf(object.rdbuf());
        }
        ~Source()
        {
            if (openMode == OpenMode::Interactive)
            {
                return;
            }
            object.close();
        }
    public:
        /*std::filesystem::path*/std::string path;
        Encoding encoding;
        OpenMode openMode;

        std::basic_fstream<wchar_t> object;
        std::basic_string_view<wchar_t> data;
    };
}
}
}

#endif // !_FLANER_LEXER_IO_HH_
