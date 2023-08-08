
#include "caliburn.h"

#include "error.h"
#include "parser.h"
#include "tokenizer.h"

#include "ast/rootmod.h"
#include "ast/ctrlstmt.h"
#include "ast/modstmts.h"

#include "cllr/cllrout.h"

#include "spirv/cllrspirv.h"

using namespace caliburn;

void Compiler::o(OptimizeLevel lvl)
{
	settings->o = lvl;
}

void Compiler::setValidationLvl(ValidationLevel lvl)
{
	settings->vLvl = lvl;
}

void Compiler::setDynamicType(std::string inner, std::string concrete)
{
	settings->dynTypes.emplace(inner, concrete);
}

std::vector<uptr<Shader>> Compiler::compileSrcShaders(std::string src, std::string shaderName)
{
	if (shaderName.length() == 0)
	{
		throw std::exception("Caliburn: passed shader name is empty!");
	}

	auto doc = TextDoc(src);
	
	//Parse the text into tokens (duh)
	auto t = Tokenizer(doc);
	auto tokens = t.tokenize();

	t.errors->printout(allErrors, doc);

	//Build the initial AST (ok)
	auto p = Parser(tokens);
	auto ast = p.parse();

	p.errors->printout(allErrors, doc);

	//Validate AST
	//keep this a shared pointer since the AST will use it to report validation errors
	auto astErrs = new_sptr<ErrorHandler>(CompileStage::AST_VALIDATION);
	for (auto const& stmt : ast)
	{
		std::set<StatementType> topStmts = TOP_STMT_TYPES;

		if (topStmts.find(stmt->type) == topStmts.end())
		{
			//TODO complain
			continue;
		}
		/*
		if (!stmt->validate(topStmts))
		{
			//TODO complain
		}
		*/
	}

	astErrs->printout(allErrors, doc);

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

	//COMPILE

	//TODO populate built-in types
	//auto root = new_sptr<RootModule>();

	ptr<ShaderStatement> shaderStmt = nullptr;

	for (auto const& stmt : ast)
	{
		//import x;
		if (stmt->type == StatementType::IMPORT)
		{
			auto imp = static_cast<ptr<ImportStatement>>(stmt.get());

			//root->importModule(imp->name->str, imp->alias == nullptr ? "" : imp->alias->str);
			continue;
		}

		//mod x;
		//So we rename the module here. Right now it does nothing.
		if (stmt->type == StatementType::MODULE)
		{
			auto modDecl = static_cast<ptr<ModuleStatement>>(stmt.get());

			continue;
		}

		//type x = y;
		if (stmt->type == StatementType::TYPEDEF)
		{

		}

		//shader x {}
		//where the real magic happens
		if (stmt->type == StatementType::SHADER)
		{
			auto shadDecl = static_cast<ptr<ShaderStatement>>(stmt.get());

			if (shadDecl->name->str != shaderName)
			{
				continue;
			}

			if (shaderStmt == nullptr)
			{
				shaderStmt = shadDecl;
			}
			else
			{
				//TODO complain
			}

			break;
		}

		break;
	}

	if (shaderStmt == nullptr)
	{
		throw std::exception((std::stringstream() << "Caliburn: Shader not found: " << shaderName).str().c_str());
	}

	auto table = new_sptr<SymbolTable>();

	/*
	for (auto const& [inner, outer] : dynTypes)
	{
		auto tokenizer = Tokenizer(outer);
		auto tkns = tokenizer.tokenize();
		auto p = Parser(tkns);

		if (auto pt = p.parseTypeName())
		{
			if (auto t = pt->resolve(table))
			{
				if (!table->add(inner, t))
				{
					//TODO complain
				}

			}
			else
			{
				//TODO complain
			}

		}
		else
		{
			//TODO complain
		}

	}
	*/
	//Declare headers
	for (auto const& stmt : ast)
	{
		stmt->declareHeader(table);
	}

	//Declare everything else
	for (auto const& stmt : ast)
	{
		stmt->declareSymbols(table);
	}

	std::vector<uptr<Shader>> shaders;
	std::vector<sptr<Error>> compileErrs;

	for (auto const& stage : shaderStmt->stages)
	{
		auto result = stage->compile(table, settings, compileErrs);

		uint32_t d = 0;

		for (auto const& desc : shaderStmt->descriptors)
		{
			result->sets.push_back(DescriptorSet{ desc.second->str, d++ });

		}

		shaders.push_back(std::move(result));

	}

	if (!compileErrs.empty())
	{
		for (auto const& e : compileErrs)
		{
			allErrors.push_back(e->toStr(doc));

		}

	}

	return shaders;
}

std::vector<std::string> Compiler::getErrors() const
{
	return allErrors;
}
