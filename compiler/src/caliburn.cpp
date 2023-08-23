
#define CBRN_NO_IMPORT
#include "caliburn.h"

#include "error.h"
#include "parser.h"
#include "tokenizer.h"

#include "ast/rootmod.h"
#include "ast/ctrlstmt.h"
#include "ast/modstmts.h"

#include "types/typearray.h"
#include "types/typebool.h"
#include "types/typefloat.h"
#include "types/typeint.h"
#include "types/typevector.h"
#include "types/typevoid.h"

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

std::vector<uptr<Shader>> Compiler::compileSrcShaders(std::string src, std::string shaderName, GPUTarget target)
{
	if (shaderName.length() == 0)
	{
		throw std::exception("Caliburn: passed shader name is empty!");
	}

	std::vector<uptr<Shader>> shaders;

	auto doc = TextDoc(src);
	
	//Parse the text into tokens (duh)
	auto t = Tokenizer(doc);
	auto tokens = t.tokenize();

	if (!t.errors->empty())
	{
		t.errors->printout(allErrors, doc);

		//TODO reconsider
		return shaders;
	}
	
	//Build the initial AST (ok)
	auto p = Parser(tokens);
	auto ast = p.parse();

	if (!p.errors->empty())
	{
		p.errors->printout(allErrors, doc);

		//TODO reconsider
		return shaders;
	}

	//Validate AST
	//keep this a shared pointer since the AST will use it to report validation errors
	/*auto astErrs = new_sptr<ErrorHandler>(CompileStage::AST_VALIDATION);
	for (auto const& stmt : ast)
	{
		std::set<StatementType> topStmts = TOP_STMT_TYPES;

		if (topStmts.find(stmt->type) == topStmts.end())
		{
			//TODO complain
			continue;
		}

		if (!stmt->validate(topStmts))
		{
			//TODO complain
		}
		
	}

	if (!astErrs->empty())
	{
		astErrs->printout(allErrors, doc);

		return shaders;
	}
	*/
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

	ptr<ShaderStatement> shaderStmt = nullptr;

	for (auto const& stmt : ast)
	{
		/*
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
		*/
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
			
			break;
		}

		break;
	}

	if (shaderStmt == nullptr)
	{
		throw std::exception((std::stringstream() << "Caliburn: Shader not found: " << shaderName).str().c_str());
	}

	shaderStmt->sortStages();

	//Populate the built-in symbols table

	auto root = new_sptr<SymbolTable>();

	root->add("array", new_sptr<TypeArray>());
	root->add("bool", new_sptr<TypeBool>());
	root->add("void", new_sptr<TypeVoid>());

	for (auto bits = MIN_INT_BITS; bits <= MAX_INT_BITS; bits *= 2)
	{
		root->add((std::stringstream() << "int" << bits).str(), new_sptr<TypeInt>(bits, true));
		root->add((std::stringstream() << "uint" << bits).str(), new_sptr<TypeInt>(bits, false));
	}

	for (auto bits = MIN_FLOAT_BITS; bits <= MAX_FLOAT_BITS; bits *= 2)
	{
		root->add((std::stringstream() << "float" << bits).str(), new_sptr<TypeFloat>(bits));
	}

	for (auto len = MIN_VECTOR_LEN; len <= MAX_VECTOR_LEN; ++len)
	{
		root->add((std::stringstream() << "vec" << len).str(), new_sptr<TypeVector>(len));
	}

	auto table = new_sptr<SymbolTable>(root);

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

	std::vector<sptr<Error>> compileErrs;
	shaders = shaderStmt->compile(table, settings, compileErrs);

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
