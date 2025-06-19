
#include <gtest/gtest.h>

#include "tokenizer.h"

using namespace caliburn;

class TokenTests : public testing::Test
{
protected:
    std::vector<Token> tokenize(in<std::string> input)
    {
        auto doc = new_sptr<TextDoc>(input);
        Tokenizer tokenizer(doc);
        return tokenizer.tokenize();
    }

    static inline void assertToken(in<Token> token, in<std::string_view> expectedStr, in<TokenType> expectedType)
    {
        EXPECT_EQ(token.str, expectedStr);
        EXPECT_EQ(token.type, expectedType);
    }
};

TEST_F(TokenTests, EmptyString)
{
    auto tokens = tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST_F(TokenTests, SimpleIdentifier)
{
    auto tokens = tokenize("hello");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "hello", TokenType::IDENTIFIER);
}

TEST_F(TokenTests, Keywords)
{
    auto tokens = tokenize("def var if");
    ASSERT_EQ(tokens.size(), 3);
    assertToken(tokens[0], "def", TokenType::KEYWORD);
    assertToken(tokens[1], "var", TokenType::KEYWORD);
    assertToken(tokens[2], "if", TokenType::KEYWORD);
}

TEST_F(TokenTests, IntegerLiterals)
{
    auto tokens = tokenize("42 0xFF 0b1010");
    ASSERT_EQ(tokens.size(), 3);
    assertToken(tokens[0], "42", TokenType::LITERAL_INT);
    assertToken(tokens[1], "0xFF", TokenType::LITERAL_INT);
    assertToken(tokens[2], "0b1010", TokenType::LITERAL_INT);
}

TEST_F(TokenTests, FloatLiterals)
{
    auto tokens = tokenize("3.14 1.0e-10 42.0f");
    ASSERT_EQ(tokens.size(), 3);
    assertToken(tokens[0], "3.14", TokenType::LITERAL_FLOAT);
    assertToken(tokens[1], "1.0e-10", TokenType::LITERAL_FLOAT);
    assertToken(tokens[2], "42.0f", TokenType::LITERAL_FLOAT);
}

TEST_F(TokenTests, StringLiterals)
{
    auto tokens = tokenize("\"hello there\" 'world'");
    ASSERT_EQ(tokens.size(), 2);

    assertToken(tokens[0], "\"hello there\"", TokenType::LITERAL_STR);
    assertToken(tokens[1], "'world'", TokenType::LITERAL_STR);
}

TEST_F(TokenTests, Operators)
{
    auto tokens = tokenize("+ || >= << &&");
    ASSERT_EQ(tokens.size(), 5);
    
    assertToken(tokens[0], "+", TokenType::OPERATOR);
    assertToken(tokens[1], "||", TokenType::OPERATOR);
    assertToken(tokens[2], ">=", TokenType::OPERATOR);
    assertToken(tokens[3], "<<", TokenType::OPERATOR);
    assertToken(tokens[4], "&&", TokenType::OPERATOR);
    
}

TEST_F(TokenTests, SpecialCharacters)
{
    auto tokens = tokenize("( ) [ ] { } ,");
    ASSERT_EQ(tokens.size(), 7);
    assertToken(tokens[0], "(", TokenType::START_PAREN);
    assertToken(tokens[1], ")", TokenType::END_PAREN);
    assertToken(tokens[2], "[", TokenType::START_BRACKET);
    assertToken(tokens[3], "]", TokenType::END_BRACKET);
    assertToken(tokens[4], "{", TokenType::START_SCOPE);
    assertToken(tokens[5], "}", TokenType::END_SCOPE);
    assertToken(tokens[6], ",", TokenType::COMMA);
}

TEST_F(TokenTests, Comments)
{
    auto tokens = tokenize("# This is a comment\nidentifier");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "identifier", TokenType::IDENTIFIER);
}

TEST_F(TokenTests, ConstStmt)
{
    auto tokens = tokenize("const x = 42 + y");
    ASSERT_EQ(tokens.size(), 6);
    assertToken(tokens[0], "const", TokenType::KEYWORD);
    assertToken(tokens[1], "x", TokenType::IDENTIFIER);
    assertToken(tokens[2], "=", TokenType::SETTER);
    assertToken(tokens[3], "42", TokenType::LITERAL_INT);
    assertToken(tokens[4], "+", TokenType::OPERATOR);
    assertToken(tokens[5], "y", TokenType::IDENTIFIER);
}

TEST_F(TokenTests, EscapedStrings)
{
    auto tokens = tokenize("\"hello\\\"world\"");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "\"hello\\\"world\"", TokenType::LITERAL_STR);
}
