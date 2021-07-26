#include <lexer.hh>

int main(int argc, char* argv[])
{
    using namespace flaner::lexer;
    std::cout << "\nFlaner Programming Language.\n--------\n\n";

    try
    {
        Lexer lexer{ std::string{ argv[1] } };

        auto tokens = lexer.getSequence();
        for (auto i : tokens)
        {
            if (i.type == Lexer::TokenType::STRING)
            {
                i.value = '"' + i.value + '"';
            }
            std::cout << "[type: " << static_cast<int>(i.type) << ", value: " << i.value << "]\n";
        }
    }
    catch (const Lexer::LexError& e)
    {
        std::cout << "Error! " << e.info << "\nline " << e.line << ", offset " << e.offset << ".";
    }
}
