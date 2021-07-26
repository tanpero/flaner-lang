#include <lexer.hh>
#include <cassert>

namespace flaner
{
namespace lexer
{
    static inline size_t it2idx(std::vector<Lexer::Token>& seq, std::vector<Lexer::Token>::iterator& it)
    {
        return static_cast<size_t>(it - seq.begin());
    }

    bool Lexer::isBlank(char ch)
    {
        std::string blanks = "\n\r\t\f \x0b\xa0\u2000"
            "\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009"
            "\u200a\u200b\u2028\u2029\u3000";
        for (auto i = blanks.begin(); i != blanks.end(); i++)
        {
            if (*i == ch)
            {
                return true;
            }
        }
        return false;
    }

    Lexer::TokenType Lexer::getKeywordOrID(std::string s)
    {
        TokenType type;

        try
        {
            type = keywordMap.at(s);
        }
        catch (const std::exception&)
        {
            type = TokenType::IDENTIFIER;
        }
        return type;
    }

    std::string Lexer::getNumber()
    {
        std::string n{};
        char ch = context.getLastchar();
        char state = 1;
        do
        {
            switch (ch)
            {
            case 'e':
            {
                if (state != 2 && state != 14)
                {
                    return n;
                }
                state = 9;
                n += 'e';
                break;
            }
            case '.':
            {
                if (state >> 2)
                {
                    return n;
                }
                state = 12;
                n += '.';
                break;
            }
            case '+':
            case '-':
            {
                if (!(state & 1))
                {
                    return n;
                }
                n += ch;
                break;
            }
            default:
                if (ch >= '0' && ch <= '9')
                {
                    if (state >> 2 == 1)
                    {
                        return n;
                    }
                    state = state & 8 ? 14 : ((state >> 1 & 2) + 2);
                    n += ch;
                }
                else
                {
                    return n;
                }
                break;
            }

            ch = context.getNextchar();

        } while (true);

        return n;
    }

    inline char Lexer::getEscapeCharacter()
    {
        char ch = context.getNextchar();
        char m = 0;

        // TODO: 对真·换行符的处理
        switch (ch)
        {
        case 'b': m = '\x08'; break;
        case 't': m = '\x09'; break;
        case 'n': m = '\x0a'; break;
        case 'v': m = '\x0b'; break;
        case 'f': m = '\x0c'; break;
        case 'r': m = '\x0d'; break;
        case '\'': m = '\x27'; break;
        case '"': m = '\x22'; break;
        case '\\': m = '\x5c'; break;

            // 处理被转义的 \r\n
        case '\r': ch = context.getNextchar();
        case '\n': ch = context.getNextchar(); break;
        default:
            m = ch; break;
        }
        return m;
    }

    std::string Lexer::getString(char mark)
    {
        std::string s{};
        char ch = context.getNextchar();

        while (true)
        {
            if (ch == mark)
            {
                if (context.lookLastchar() != '\\')
                {
                    return s;
                }
                s += ch;
            }
            else
            {
                if (ch == '\\')
                {
                    s += getEscapeCharacter();
                }
                else if (ch == '\r' || ch == '\n')
                {
                   error("Invalid or unexpected token");
                }
                else
                {
                    s += ch;
                }
            }
            ch = context.getNextchar();
        }
    }

    void Lexer::processTemplateString(std::function<void(TokenType, std::string)> push)
    {
        if (context.getLastchar() == '}')
        {
            push(TokenType::OP_PAREN_END, ")");
            push(TokenType::OP_ADD, "+");
            levelOfTemplateNesting -= 1;
        }

        char ch = context.getNextchar();
        std::string s{};

        while (true)
        {
            if (ch == '\\')
            {
                s += getEscapeCharacter();
            }
            else if (ch == '$')
            {
                ch = context.getNextchar();
                if (ch == '{')
                {
                    push(TokenType::STRING, s);
                    push(TokenType::OP_ADD, "+");
                    push(TokenType::OP_PAREN_BEGIN, "(");
                    levelOfTemplateNesting += 1;
                    return;
                }
                else
                {
                    s += ch;
                }
            }
            else if (ch == '`')
            {
                push(TokenType::STRING, s);
                return;
            }
            else
            {
                s += ch;
            }

            ch = context.getNextchar();
        }
    }

    void Lexer::process()
    {
        auto push = [&](TokenType t, std::string v) {
			try
			{
				sequence.push_back({ t, v });
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
				abort();
			}
        };
        auto next = [&](size_t offset = 1) {
            return context.getNextchar(offset);
        };
        auto lastToken = [&]() {
            return sequence.at(sequence.size() - 1);
        };

        levelOfTemplateNesting = 0;
        levelOfParanthesesNestingInTemplateInnerEvaluation = 0;

        while (!context.isEnd())
        {
            char ch = next();

            if (isBlank(ch))
            {
                continue;
            }

            auto match = [&](char s) {
                return ch == s;
            };
            auto test = [&](char s, size_t offset = 1) {
                return context.lookNextchar(offset) == s;
            };

            if (isdigit(ch) || (ch == '.' && isdigit(context.lookNextchar())))
            {
                push(TokenType::NUMBER, getNumber());
            }      
            else if (isalpha(ch) || ch == L'_' || ch == L'$')
            {
                std::string word{ ch };
                char nextchar = context.lookNextchar(1);
                while (isalnum(nextchar) || ch == L'_' || ch == L'$')
                {
                    ch = next();
                    nextchar = context.lookNextchar(1);
                    word += ch;
                }
                if (sequence.size() != 0 && sequence.back().type == TokenType::OP_DOT)
                {
                    push(TokenType::IDENTIFIER, word);
                }
                else
                {
                    push(getKeywordOrID(word), word);
                }
            }
            else if (match('\''))
            {
                push(TokenType::STRING, getString('\''));
            }
            else if (match('"'))
            {
                push(TokenType::STRING, getString('"'));
            }
            else if (match('`'))
            {
                processTemplateString(push);
            }
            else if (match('+'))
            {
                if (test('='))
                {
                    push(TokenType::OP_ADD_ASSIGN, "+=");
                    next();
                }
                else
                {
                    push(TokenType::OP_ADD, "+");
                }
            }
            else if (match('-'))
            {
                if (test('='))
                {
                    push(TokenType::OP_MINUS_ASSIGN, "-=");
                    next();
                }
                else
                {
                    push(TokenType::OP_MINUS, "-");
                }
            }
            else if (match('*'))
            {
                if (test('*'))
                {
                    push(TokenType::OP_POW, "**");
                    next();
                }
                else
                {
                    if (test('='))
                    {

                        push(TokenType::OP_MUL_ASSIGN, "*=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_MUL, "*");
                    }
                    next();
                }
            }
            else if (match('/'))
            {
                if (test('/'))
                {
                    push(TokenType::OP_INTDIV_ASSIGN, "//");
                    next();
                }
                else
                {
                    if (test('='))
                    {
                        push(TokenType::OP_DIV_ASSIGN, "/=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_DIV, "/");
                    }
                }
            }
            else if (match('%'))
            {
                if (test('%'))
                {
                    push(TokenType::OP_QUOTE, "%%");
                    next();
                }
                else
                {
                    if (test('='))
                    {

                        push(TokenType::OP_MOD_ASSIGN, "%=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_MOD, "%");
                    }
                    next();
                }
            }
            else if (match('|'))
            {
                if (test('|'))
                {
                    push(TokenType::OP_LOGIC_OR, "||");
                    next();
                }
                else
                {
                    if (test('='))
                    {
                        push(TokenType::OP_BIT_OR_ASSIGN, "|=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_BIT_OR, "|");
                    }
                }
            }
            else if (match('&'))
            {
                if (test('&'))
                {
                    push(TokenType::OP_LOGIC_AND, "&&");
                    next();
                }
                else
                {
                    if (test('='))
                    {
                        push(TokenType::OP_BIT_OR_ASSIGN, "&=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_BIT_OR, "&");
                    }
                }
            }
            else if (match('<'))
            {
                if (test('<'))
                {
                    push(TokenType::OP_SHIFT_LEFT, "<<");
                    next();
                }
                else
                {
                    if (test('='))
                    {
                        push(TokenType::OP_LESS_EQUAL, "<=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_LESS_THAN, "<");
                    }
                }
            }
            else if (match('>'))
            {
                if (test('>'))
                {
                    push(TokenType::OP_SHIFT_RIGHT, ">>");
                    next();
                }
                else
                {
                    if (test('='))
                    {
                        push(TokenType::OP_GREATER_EQUAL, ">=");
                        next();
                    }
                    else
                    {
                        push(TokenType::OP_GREATER_THAN, ">");
                    }
                }
            }
            else if (match('='))
            {
                bool pureAssignment = true;
                auto replace = [&](TokenType t1, TokenType t2, std::string s) {
                    if (sequence.size() != 0 && sequence.back().type == t1)
                    {
                        sequence.pop_back();
                        push(t2, s);
                        pureAssignment = false;
                    }
                };

                if (test('>'))
                {
                    push(TokenType::FUNCTION_ARROW, "=>");
                    next();
                    continue;
                }

                replace(TokenType::OP_POW, TokenType::OP_POW_ASSIGN, "**=");
                replace(TokenType::OP_QUOTE, TokenType::OP_QUOTE_ASSIGN, "%%=");
                replace(TokenType::OP_INTDIV, TokenType::OP_INTDIV_ASSIGN, "//=");
                replace(TokenType::OP_SHIFT_LEFT, TokenType::OP_SHIFT_LEFT_ASSIGN, "<<=");
                replace(TokenType::OP_SHIFT_RIGHT, TokenType::OP_SHIFT_RIGHT_ASSIGN, ">>=");

                if (pureAssignment)
                {
                    push(TokenType::OP_ASSIGN, "=");
                }

            }
            else if (match('('))
            {
                push(TokenType::OP_PAREN_BEGIN, "(");
            }
            else if (match(')'))
            {
                push(TokenType::OP_PAREN_END, ")");
            }
            else if (match('['))
            {
                push(TokenType::OP_BRACKET_BEGIN, "[");
            }
            else if (match(']'))
            {
                push(TokenType::OP_BRACKET_END, "]");
            }
            else if (match('{'))
            {
                if (levelOfTemplateNesting > 0)
                {
                    levelOfParanthesesNestingInTemplateInnerEvaluation += 1;
                }
                push(TokenType::OP_BRACE_BEGIN, "{");
            }
            else if (match('}'))
            {
                if (levelOfParanthesesNestingInTemplateInnerEvaluation > 0)
                {
                    levelOfParanthesesNestingInTemplateInnerEvaluation -= 1;
                }
                if (levelOfParanthesesNestingInTemplateInnerEvaluation == 0)
                {
                    processTemplateString(push);
                }
                else
                {
                    push(TokenType::OP_BRACE_END, "}");
                }
            }
            else if (match('.'))
            {
                if (sequence.size() != 0 && sequence.back().type == TokenType::OP_DOT_DOT)
                {
                    sequence.pop_back();
                    push(TokenType::OP_DOT_DOT_DOT, "...");
                }
                else if (test('.'))
                {
                    push(TokenType::OP_DOT_DOT, "..");
                    next();
                }
                else
                {
                    push(TokenType::OP_DOT, ".");
                }
            }
            else if (match(':'))
            {
                push(TokenType::OP_COLON, ":");
            }
            else if (match(','))
            {
                push(TokenType::OP_COMMA, ",");
            }
            else if (match('?'))
            {
                push(TokenType::OP_QUESTION, "?");
            }
            else if (match(';'))
            {
                push(TokenType::OP_SEMICOLON, ";");
            }
            else
            {
                if (ch == EOF)
                {
                    push(TokenType::END_OF_FILE, { ch });
                }
                else
                {
                    push(TokenType::UNKNOWN, { ch });
                }
            }
        }

        levelOfTemplateNesting = 0;
    }

    std::vector<Lexer::Token> Lexer::getSequence()
    {
        return std::move(sequence);
    }

    Lexer::Token Lexer::forwards(size_t n)
    {
        auto offset = location + n;
        if (it2idx(sequence, offset) >= sequence.size())
        {
            return { TokenType::END_OF_FILE, { EOF } };
        }
        return *offset;
    }
    Lexer::Token Lexer::backwards(size_t n)
    {
        if (it2idx(sequence, location) < n)
        {
            // TODO...
        }
        return *(location - n);
    }
    Lexer::Token Lexer::go(size_t n)
    {
        location += n;
        return *location;
    }
    Lexer::Token Lexer::last(size_t n)
    {
        location -= n;
        return *location;
    }
    Lexer::Token Lexer::now()
    {
        assert(sequence.size());
        return sequence.at(0);
    }

    size_t Lexer::tryFindingAfter(std::unordered_set<TokenType> patterns, TokenType t1, TokenType t2)
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        for (auto i = location; i != sequence.end() && patterns.find(*i) != patterns.end(); ++i)
        {
            if (*i == t1)
            {
                if (*(++i) == t2)
                {
                    return std::distance(location, i);
                }
                else
                {
                    return 0;
                }
            }
        }
        return 0;
    }
    
    size_t Lexer::tryFinding(std::unordered_set<TokenType> patterns, TokenType t)
    {
        for (auto i = location; i != sequence.end() && patterns.find(*i) != patterns.end(); ++i)
        {
            if (*i == t)
            {
                return std::distance(location, i);
            }
        }
        return 0;
    }

    bool Lexer::isEnd()
    {
        return cursor >= sequence.size();
    }
    std::unordered_map<std::string, Lexer::TokenType> Lexer::getKeywordMap()
    {
        return keywordMap;
    }
    std::unordered_set<Lexer::TokenType> Lexer::getOperatorSet()
    {
        return operatorSet;
    }
    void Lexer::error(std::string info)
    {
        throw LexError{ "SyntaxError: " + info, context.lineOffset, context.charOffset };
    }
}
}
