#include <context.hh>

namespace flaner
{
    namespace lexer
    {
        char Context::thischar()
        {
            return source.object.peek();
        }

        char Context::getNextchar(size_t offset)
        {
            if (offset == 1)
            {
                return source.object.get();
            }
            else
            {
                source.object.seekp((offset - 1) * sizeof(char), std::ios::cur);
                return source.object.get();
            }
        }
        char Context::lookNextchar(size_t offset)
        {
            if (offset == 1)
            {
                return source.object.peek();
            }
            else
            {
                source.object.seekp((offset - 1) * sizeof(char), std::ios::cur);
                char ch = source.object.get();
                source.object.seekp(-static_cast<int>(offset * sizeof(char)), std::ios::cur);
                return ch;
            }
        }
        char Context::getLastchar()
        {
            source.object.unget();
            return getNextchar();
        }
        char Context::lookLastchar()
        {
            char ch = getLastchar();
            getNextchar();
            return ch;
        }
        bool Context::isEnd()
        {
            return lookNextchar(1) == EOF;
        }
    }
}
