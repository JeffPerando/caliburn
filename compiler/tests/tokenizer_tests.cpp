
#include <gtest/gtest.h>

#include "tokenizer.h"

using namespace caliburn;

#define CBRN_TEST_TOKENIZE(str) auto doc = new_sptr<TextDoc>(str); Tokenizer tokenizer(doc); auto tokens = tokenizer.tokenize();

static inline void assertToken(in<Token> token, in<std::string_view> expectedStr, in<TokenType> expectedType)
{
    EXPECT_EQ(token.str, expectedStr);
    EXPECT_EQ(token.type, expectedType);
}

TEST(TokenTests, EmptyString)
{
    CBRN_TEST_TOKENIZE("");
    EXPECT_TRUE(tokens.empty());
}

TEST(TokenTests, SimpleIdentifier)
{
    CBRN_TEST_TOKENIZE("hello");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "hello", TokenType::IDENTIFIER);
}

TEST(TokenTests, NumFirstIdentifier)
{
    CBRN_TEST_TOKENIZE("2DTexture");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "2DTexture", TokenType::IDENTIFIER);
}

TEST(TokenTests, Keywords)
{
    CBRN_TEST_TOKENIZE("def var if");
    ASSERT_EQ(tokens.size(), 3);
    assertToken(tokens[0], "def", TokenType::KEYWORD);
    assertToken(tokens[1], "var", TokenType::KEYWORD);
    assertToken(tokens[2], "if", TokenType::KEYWORD);
}

TEST(TokenTests, IntegerLiterals)
{
    CBRN_TEST_TOKENIZE("42 0xFF 0b1010");
    ASSERT_EQ(tokens.size(), 3);
    assertToken(tokens[0], "42", TokenType::LITERAL_INT);
    assertToken(tokens[1], "0xFF", TokenType::LITERAL_INT);
    assertToken(tokens[2], "0b1010", TokenType::LITERAL_INT);
}

TEST(TokenTests, FloatLiterals)
{
    CBRN_TEST_TOKENIZE("1f 3.14 1.0e-10 42.0f");
    ASSERT_EQ(tokens.size(), 4);
    assertToken(tokens[0], "1f", TokenType::LITERAL_FLOAT);
    assertToken(tokens[1], "3.14", TokenType::LITERAL_FLOAT);
    assertToken(tokens[2], "1.0e-10", TokenType::LITERAL_FLOAT);
    assertToken(tokens[3], "42.0f", TokenType::LITERAL_FLOAT);
}

TEST(TokenTests, StringLiterals)
{
    CBRN_TEST_TOKENIZE("\"hello there\" 'world'");
    ASSERT_EQ(tokens.size(), 2);

    assertToken(tokens[0], "\"hello there\"", TokenType::LITERAL_STR);
    assertToken(tokens[1], "'world'", TokenType::LITERAL_STR);
}

TEST(TokenTests, Operators)
{
    CBRN_TEST_TOKENIZE("+ || >= << &&");
    ASSERT_EQ(tokens.size(), 5);
    
    assertToken(tokens[0], "+", TokenType::OPERATOR);
    assertToken(tokens[1], "||", TokenType::OPERATOR);
    assertToken(tokens[2], ">=", TokenType::OPERATOR);
    assertToken(tokens[3], "<<", TokenType::OPERATOR);
    assertToken(tokens[4], "&&", TokenType::OPERATOR);
    
}

TEST(TokenTests, SpecialCharacters)
{
    CBRN_TEST_TOKENIZE("( ) [ ] { } ,");
    ASSERT_EQ(tokens.size(), 7);
    assertToken(tokens[0], "(", TokenType::START_PAREN);
    assertToken(tokens[1], ")", TokenType::END_PAREN);
    assertToken(tokens[2], "[", TokenType::START_BRACKET);
    assertToken(tokens[3], "]", TokenType::END_BRACKET);
    assertToken(tokens[4], "{", TokenType::START_SCOPE);
    assertToken(tokens[5], "}", TokenType::END_SCOPE);
    assertToken(tokens[6], ",", TokenType::COMMA);
}

TEST(TokenTests, Comments)
{
    CBRN_TEST_TOKENIZE("# This is a comment\nidentifier");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "identifier", TokenType::IDENTIFIER);
}

TEST(TokenTests, ConstStmt)
{
    CBRN_TEST_TOKENIZE("const x = 42 + y;");
    ASSERT_EQ(tokens.size(), 7);
    assertToken(tokens[0], "const", TokenType::KEYWORD);
    assertToken(tokens[1], "x", TokenType::IDENTIFIER);
    assertToken(tokens[2], "=", TokenType::SETTER);
    assertToken(tokens[3], "42", TokenType::LITERAL_INT);
    assertToken(tokens[4], "+", TokenType::OPERATOR);
    assertToken(tokens[5], "y", TokenType::IDENTIFIER);
    assertToken(tokens[6], ";", TokenType::END);
}

TEST(TokenTests, EscapedStrings)
{
    CBRN_TEST_TOKENIZE("\"hello\\\"world\"");
    ASSERT_EQ(tokens.size(), 1);
    assertToken(tokens[0], "\"hello\\\"world\"", TokenType::LITERAL_STR);
}

TEST(TokenTests, SmallShader)
{
    CBRN_TEST_TOKENIZE("type FP = dynamic<fp32>; shader TestShader{ vec4 frag_color; def vertex(vec4<FP> v, vec4 c) : vec4<FP> {frag_color = c;return v;}; def frag() : vec4 {return frag_color;};};");
    ASSERT_EQ(tokens.size(), 55);
    //no, we're not testing every token in the file, just pick one and call it a day.
    assertToken(tokens[49], "frag_color", TokenType::IDENTIFIER);
}
