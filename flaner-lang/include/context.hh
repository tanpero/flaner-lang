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

        char thischar();
        char getNextchar(size_t offset = 1);
        char lookNextchar(size_t offset = 1);
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
