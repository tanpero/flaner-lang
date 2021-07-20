#ifndef _FLANER_LEXER_CONTEXT_HH_
#define _FLANER_LEXER_CONTEXT_HH_

#include <io.hh>

namespace flaner
{
namespace lexer
{
    class Context
    {
    public:
        Context(std::string path)
            : source(path),
            lineOffset(0), charOffset(0)
        {

        }

        Context(std::wstreambuf* buf)
            : source(buf),
            lineOffset(0), charOffset(0)
        {

        }

        Context(const Context& c)
            : source(c.source),
            lineOffset(c.lineOffset), charOffset(c.charOffset)
        {

        }


        ~Context() {}

    public:

        // offset 为 1 时获取当前字符
        char getNextchar(size_t offset);
        char lookNextchar(size_t offset);
        char getLastchar();
        char lookLastchar();
        bool isEnd();

    public:
        io::Source source;
        size_t lineOffset, charOffset;
    };
}
}

#endif // !_FLANER_LEXER_CONTEXT_HH_
