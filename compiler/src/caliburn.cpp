
#define CBRN_NO_IMPORT
#include "caliburn.h"

#include "error.h"
#include "parser.h"
#include "tokenizer.h"

#include "ast/rootmod.h"
#include "ast/ctrlstmt.h"
#include "ast/modstmts.h"
#include "ast/stdlib.h"

using namespace caliburn;

ShaderResult Compiler::compileSrcShaders(const std::string& src, const std::string& shaderName)
{
	ShaderResult result;

	if (shaderName.length() == 0)
	{
		result.errors.push_back("Passed shader name is empty!");
		return result;
	}

	auto doc = new_sptr<TextDoc>(src);

	auto t = Tokenizer(doc);
	auto tokens = t.tokenize();

	auto p = Parser(settings, tokens);
	auto ast = p.parse();

	if (!p.errors->empty())
	{
		p.errors->printout(result.errors, *doc);
		return result;
	}

	//TODO AST validation and conditional compilation go here

	//COMPILE

	ptr<ShaderStmt> shaderStmt = nullptr;

	for (auto const& stmt : ast)
	{
		//where the real magic happens
		if (stmt->type == ExprType::SHADER)
		{
			auto shadDecl = RCAST<ptr<ShaderStmt>>(stmt.get());

			if (shadDecl->name.str != shaderName)
			{
				continue;
			}

			if (shaderStmt == nullptr)
			{
				shaderStmt = shadDecl;
			}

			break;
		}

	}

	if (shaderStmt == nullptr)
	{
		result.errors.push_back(((std::stringstream() << "Shader not found: " << shaderName).str()));
		return result;
	}

	//Populate the built-in symbols table
	auto root = makeStdLib(settings);

	auto table = new_sptr<SymbolTable>(root);

	auto symErr = ErrorHandler(CompileStage::SYMBOL_GENERATION, settings);

	//Declare headers
	for (auto const& stmt : ast)
	{
		stmt->declareHeader(table, symErr);
	}

	if (!symErr.empty())
	{
		symErr.printout(result.errors, *doc);
		return result;
	}

	shaderStmt->compile(table, settings, result, *doc);

	return result;
}
