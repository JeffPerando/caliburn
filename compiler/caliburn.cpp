
#include "caliburn.h"
#include "parser.h"
#include "tokenizer.h"

#include "rootmod.h"
#include "ctrlstmt.h"
#include "modstmts.h"

using namespace caliburn;

std::vector<Statement*>* parseSrc(std::string text)
{
	std::vector<Token> tokens;
	auto ast = new std::vector<Statement*>();

	//Parse the text into tokens (duh)
	Tokenizer t;
	t.tokenize(text, tokens);

	//Build the initial AST (ok)
	Parser p;
	p.parse(&tokens, ast);

	//Validate AST
	for (auto stmt : *ast)
	{
		std::set<StatementType> topStmts = TOP_STMT_TYPES;

		if (topStmts.find(stmt->type) == topStmts.end())
		{
			//TODO throw compiler error
			continue;
		}
		/*
		if (!stmt->validate(topStmts))
		{
			//TODO throw compiler error
		}
		*/
	}

	return ast;
}

std::vector<Statement*>* parseCBIR(std::vector<uint32_t>* cbir)
{
	return nullptr;
}

bool compileShaders(std::vector<Statement*>* ast, std::string shaderName, std::vector<Shader>& shaderDest)
{
	/*
	OK so I'll admit the code below looks utterly goofy, and is probably a form of lambda abuse.

	The next step is to conditionally compile the ast-> We do this by looking for top-level if statements.
	Then, we check its condition and evaluate, which means inserting the entirety of the branch's contents
	into the ast-> Then we sort the AST, go back to the beginning, and do it all over again. This repeats
	until there's no more if statements. Once that's done, the real compilation begins.
	*/

	/* Conditional compilation
	auto sortAST = [&ast]() -> void {
		std::sort(ast->begin(), ast->end(), [ast](const Statement* a, const Statement* b) -> bool
			{
				return a->type > b->type;
			});
	};

	auto insertCode = [&ast, sortAST](ScopeStatement* stmt) -> void {
		auto inner = &stmt->stmts;
		ast->insert(ast->end(), inner->begin(), inner->end());
		sortAST();
	};

	sortAST();

	for (auto i = 0; i < ast->size(); ++i)
	{
		auto stmt = ast->at(i);

		TODO compile-time evalation
		if (stmt->type == StatementType::VARIABLE)
		{
			continue;
		}

		if (stmt->type != StatementType::IF)
		{
			break;
		}

		auto& cond = *(IfStatement*)stmt;

		if (cond.condition->isCompileTimeConst())
		{
			if (cond.condition->eval())
			{
				insertCode(cond.ifBranch);
			}
			else
			{
				if (cond.elseBranch != nullptr)
				{
					insertCode(cond.elseBranch);
				}

			}

			i = 0;
			continue;
		}
		else
		{
			//TODO compiler error
		}

	}
	*/
	auto root = new RootModule();
	auto mod = new CompiledModule();

	//COMPILE
	for (auto stmt : *ast)
	{
		if (stmt->type == StatementType::IMPORT)
		{
			auto& imp = *(ImportStatement*)stmt;

			root->importModule(imp.name->str, imp.alias == nullptr ? "" : imp.alias->str);
			continue;
		}

		if (stmt->type == StatementType::MODULE)
		{
			auto modDecl = (ModuleStatement*)stmt;

			mod->name = modDecl->name->str;
			continue;
		}

	}

	delete mod;
	delete root;

	delete ast;

	return false;
}

bool Compiler::compileShadersSrc(std::string text, std::string shaderName, std::vector<Shader>& shaderDest)
{
	return compileShaders(parseSrc(text), shaderName, shaderDest);
}

bool Compiler::compileShadersCBIR(std::vector<uint32_t>* cbir, std::string shaderName, std::vector<Shader>& shaderDest)
{
	return compileShaders(parseCBIR(cbir), shaderName, shaderDest);
}

Compiler* Compiler::o(OptimizeLevel lvl)
{
	optimizeLvl = lvl;
	return this;
}

Compiler* Compiler::setDynamicType(std::string inner, std::string concrete)
{
	dynTypes.emplace(inner, concrete);
	return this;
}
