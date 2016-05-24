#pragma once

#include "stdafx.h"

class SyntaxError : public std::runtime_error
{
public:
    int line, column;
    explicit SyntaxError(const char* msg, int line, int column)
        : std::runtime_error(msg)
        , line(line)
        , column(column)
    {
    }
};

class Tokenizer
{
public:
    enum class Kind
    {
        Identifier,
        Number,
        String,
        LBracket,
        RBracket,
        LT,
        GT,
        LE,
        GE,
        EQ,
        Dot,
        Comma,
        SemiColon,
        Create,
        Delete,
        Values,
        Use,
        Insert,
        From,
        Like,
        Select,
        Where,
        Into,
        Exit,
        End,
    };
    struct Token
    {
        Kind kind;
        int line, column;
        std::string content;
    };

    Tokenizer(const std::string& str);

    void reset(const std::string& str = std::string());

private:
    enum State
    {
        None,
        InWord,
        InInt,
        InFloat,
        InLessThanOperator,
        InGreaterThanOperator,
        InSingleQuote,
        InDoubleQuote
    };
    State _state;
    int _line, _column;
    int _tokenLine, _tokenColumn;
    std::string _current, _content;
    std::size_t _front;
    std::size_t _end;
    std::vector<Token> _tokens;

    std::vector<Token>::iterator _nextToken;

    static std::map<std::string, Kind> _keywordMap;

    void generate_all();

    Token get()
    {

    }

    static Kind get_kind(const std::string& str)
    {
        if (str.size() == 0)
        {
            return Kind::End;
        }
        auto place = _keywordMap.find(str);
        if(place != _keywordMap.end())
        {
            return place->second;
        }
        return Kind::Identifier;
    }
    void push_token(Kind kind, char ch)
    {
        push_token(kind, std::string(1, ch));
    }
    void push_token(const std::string& str)
    {
        push_token(get_kind(str), str);
    }
    void push_token(Kind kind, const std::string& str)
    {
        _tokens.push_back(Token{ kind, _tokenLine, _tokenColumn, str });
        _current.clear();
    }
    void push_token(Kind kind)
    {
        push_token(kind, _current);
    }
    void push_token()
    {
        push_token(get_kind(_current), _current);
    }
    void push_token(char ch)
    {
        push_token(std::string(1, ch));
    }
    void assign_pos()
    {
        _tokenLine = _line;
        _tokenColumn = _column;
    }
    void push_char(char ch)
    {
        _current.push_back(ch);
    }
};
