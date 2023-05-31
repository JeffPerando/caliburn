
#include "caliburn.h"
#include "parser.h"
#include "tokenizer.h"

#include "rootmod.h"
#include "ctrlstmt.h"
#include "modstmts.h"

#include "cllrout.h"
#include "cllrspirv.h"

using namespace caliburn;

void Compiler::parseText(std::string text)
{
	if (!ast.empty())
	{
		throw std::exception("Caliburn: parsing method called a second time");
	}

	std::vector<sptr<Token>> tokens;
	
	//Parse the text into tokens (duh)
	Tokenizer t;
	t.tokenize(text, tokens);

	//Build the initial AST (ok)
	Parser p;
	p.parse(tokens, ast);

	//Validate AST
	for (auto const& stmt : ast)
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

}

void Compiler::parseCBIR(ref<std::vector<uint32_t>> cbir)
{

}

bool Compiler::compileShaders(std::string shaderName, ref<std::vector<Shader>> shaderDest)
{
	if (ast.empty())
	{
		throw std::exception("Caliburn: parsing method not called!");
	}

	/*
	OK so I'll admit the code below looks utterly goofy, and is probably a form of lambda abuse.

	The next step is to conditionally compile the AST. We do this by looking for top-level if statements.
	Then, we check its condition and evaluate, which means inserting the entirety of the branch's contents
	into the AST. Then we sort the AST, go back to the beginning, and do it all over again. This repeats
	until there's no more if statements. Once that's done, the real compilation begins.
	*/

	//Conditional compilation
	/*
	auto sortAST = lambda() {
		std::sort(ast.begin(), ast.end(), lambda(const uptr<Statement> a, const uptr<Statement> b)
			{
				return a->type > b->type;
			});
	};

	auto insertCode = lambda(uptr<ScopeStatement> stmt) {
		auto inner = &stmt->stmts;
		ast.insert(ast.end(), inner->begin(), inner->end());
		sortAST();
	};

	sortAST();

	for (auto i = 0; i < ast.size(); ++i)
	{
		auto stmt = ast.at(i);

		//TODO compile-time evalation
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
	auto root = std::make_unique<RootModule>();
	auto mod = std::make_unique<CompiledModule>();

	std::vector<ptr<ShaderStatement>> shaderDecls;

	ptr<ShaderStatement> shaderStmt = nullptr;

	//COMPILE
	for (auto const& stmt : ast)
	{
		if (stmt->type == StatementType::IMPORT)
		{
			auto imp = static_cast<ptr<ImportStatement>>(stmt.get());

			root->importModule(imp->name->str, imp->alias == nullptr ? "" : imp->alias->str);
			continue;
		}

		if (stmt->type == StatementType::MODULE)
		{
			auto modDecl = static_cast<ptr<ModuleStatement>>(stmt.get());

			mod->name = modDecl->name->str;
			continue;
		}

		if (stmt->type == StatementType::SHADER)
		{
			auto shadDecl = static_cast<ptr<ShaderStatement>>(stmt.get());

			shaderDecls.push_back(shadDecl);

			if (shadDecl->name->str == shaderName)
			{
				if (shaderStmt == nullptr)
				{
					shaderStmt = shadDecl;
				}
				else
				{
					//TODO complain
					break;
				}
			}

			continue;
		}

	}

	if (shaderStmt == nullptr)
	{
		//TODO complain
		return false;
	}

	std::vector<uptr<cllr::CompilationUnit>> shaderUnits;

	shaderStmt->compile(nullptr, shaderUnits, nullptr);

	auto outAsm = std::make_unique<cllr::SPIRVOutAssembler>();

	for (auto const& unit : shaderUnits)
	{
		if (unit->target != cllr::Target::GPU)
		{
			//TODO complain
			continue;
		}

		Shader shader;
		
		auto spirvShader = outAsm->translateCLLR(*(unit->code));

		shader.spirv = *spirvShader;


	}


	return true;
}

void Compiler::o(OptimizeLevel lvl)
{
	optimizeLvl = lvl;
}

void Compiler::setDynamicType(std::string inner, std::string concrete)
{
	dynTypes.emplace(inner, concrete);
}
