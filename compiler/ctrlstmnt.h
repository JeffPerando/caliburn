
#pragma once

#include "ast.h"

namespace caliburn
{
	struct IfStatement : public Statement
	{
		ValueStatement* condition = nullptr;
		Statement* ifBranch = nullptr;
		Statement* elseBranch = nullptr;

		IfStatement() : Statement(StatementType::IF) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			uint32_t condSSA = condition->SPIRVEmit(codeAsm, syms);
			uint32_t ifSSA = codeAsm->newAssign();
			uint32_t endSSA = codeAsm->newAssign();
			uint32_t elseSSA = endSSA;

			if (elseBranch)
			{
				elseSSA = codeAsm->newAssign();
			}

			codeAsm->pushAll({
				spirv::OpSelectionMerge(), endSSA, 0,
				spirv::OpBranchConditional(0), condSSA, ifSSA, elseSSA,
				spirv::OpLabel(), ifSSA
				});

			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = ifSSA;

			ifBranch->SPIRVEmit(codeAsm, syms);

			codeAsm->endScope();
			
			codeAsm->push(spirv::OpBranch());
			codeAsm->push(endSSA);

			if (elseBranch)
			{
				codeAsm->push(spirv::OpLabel());
				codeAsm->push(elseSSA);

				codeAsm->startScope();
				codeAsm->getCurrentScope()->label = elseSSA;

				elseBranch->SPIRVEmit(codeAsm, syms);

				codeAsm->endScope();

				codeAsm->push(spirv::OpBranch());
				codeAsm->push(endSSA);

			}
			
			codeAsm->push(spirv::OpLabel());
			codeAsm->push(endSSA);

			return 0;
		}

	};

	struct ForStatement : public Statement
	{
		Statement* preLoop = nullptr;
		ValueStatement* cond = nullptr;
		Statement* postLoop = nullptr;
		Statement* loop = nullptr;

		ForStatement() : Statement(StatementType::FOR) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			uint32_t mergeSSA = codeAsm->newAssign();
			uint32_t contSSA = codeAsm->newAssign();
			uint32_t startSSA = codeAsm->newAssign();
			uint32_t bodySSA = codeAsm->newAssign();
			uint32_t loopSSA = codeAsm->newAssign();

			//Start point
			//for
			codeAsm->pushAll({
				spirv::OpBranch(), startSSA,
				spirv::OpLabel(), startSSA });

			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = startSSA;

			//(int i = 0;
			preLoop->SPIRVEmit(codeAsm, syms);

			codeAsm->pushAll({
				spirv::OpLoopMerge(0), mergeSSA, contSSA, 0, //TODO base flag off optimize level
				//workaround for OpLoopMerge needing a branch op after
				spirv::OpBranch(), bodySSA,
				spirv::OpLabel(), bodySSA });
			
			//i < j;
			uint32_t condSSA = cond->SPIRVEmit(codeAsm, syms);

			//branch
			codeAsm->pushAll({
				spirv::OpBranchConditional(0), condSSA, loopSSA, mergeSSA,
				//{
				spirv::OpLabel(), loopSSA });

			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = loopSSA;

			loop->SPIRVEmit(codeAsm, syms);

			//loopSSA
			codeAsm->endScope();

			codeAsm->pushAll({
				spirv::OpBranch(), contSSA,
				spirv::OpLabel(), contSSA });
			//}
			//i++)
			postLoop->SPIRVEmit(codeAsm, syms);

			codeAsm->pushAll({
				//loop back
				spirv::OpBranch(), startSSA,
				//merge point
				spirv::OpLabel(), mergeSSA });

			//startSSA
			codeAsm->endScope();

			return startSSA;
		}

	};

	struct WhileStatement : public Statement
	{
		ValueStatement* cond = nullptr;
		Statement* loop = nullptr;

		WhileStatement() : Statement(StatementType::WHILE) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			uint32_t mergeSSA = codeAsm->newAssign();
			uint32_t contSSA = codeAsm->newAssign();
			uint32_t startSSA = codeAsm->newAssign();
			uint32_t bodySSA = codeAsm->newAssign();
			uint32_t loopSSA = codeAsm->newAssign();

			//while
			codeAsm->pushAll({
				spirv::OpBranch(), startSSA,
				spirv::OpLabel(), startSSA,
				//for
				spirv::OpLoopMerge(0), mergeSSA, contSSA, 0, //TODO base flag off optimize level
				//workaround for OpLoopMerge needing a branch op after
				spirv::OpBranch(), bodySSA,
				spirv::OpLabel(), bodySSA });

			// true
			uint32_t condSSA = cond->SPIRVEmit(codeAsm, syms);

			//branch
			codeAsm->pushAll({
				spirv::OpBranchConditional(0), condSSA, loopSSA, mergeSSA,
				//{
				spirv::OpLabel(), loopSSA });

			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = loopSSA;

			loop->SPIRVEmit(codeAsm, syms);

			codeAsm->endScope();

			codeAsm->pushAll({
				spirv::OpBranch(), contSSA,
				spirv::OpLabel(), contSSA });
			//}
			
			codeAsm->pushAll({
				//loop back
				spirv::OpBranch(), startSSA,
				//merge point
				spirv::OpLabel(), mergeSSA });

			return startSSA;
		}

	};

	struct DoWhileStatement : public Statement
	{
		ValueStatement* cond = nullptr;
		Statement* loop = nullptr;

		DoWhileStatement() : Statement(StatementType::DOWHILE) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			uint32_t mergeSSA = codeAsm->newAssign();
			uint32_t contSSA = codeAsm->newAssign();
			uint32_t startSSA = codeAsm->newAssign();
			uint32_t bodySSA = codeAsm->newAssign();
			uint32_t loopSSA = codeAsm->newAssign();

			codeAsm->pushAll({
				spirv::OpBranch(), loopSSA,
				spirv::OpLabel(), startSSA,
				//for
				spirv::OpLoopMerge(0), mergeSSA, contSSA, 0, //TODO base flag off optimize level
				//workaround for OpLoopMerge needing a branch op after
				spirv::OpBranch(), bodySSA,
				spirv::OpLabel(), bodySSA });

			uint32_t condSSA = cond->SPIRVEmit(codeAsm, syms);

			codeAsm->pushAll({
				spirv::OpBranchConditional(0), condSSA, loopSSA, mergeSSA,
				spirv::OpLabel(), loopSSA });

			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = loopSSA;

			loop->SPIRVEmit(codeAsm, syms);

			codeAsm->endScope();

			codeAsm->pushAll({
				spirv::OpBranch(), contSSA,
				spirv::OpLabel(), contSSA });
			
			codeAsm->pushAll({
				spirv::OpBranch(), startSSA,
				spirv::OpLabel(), mergeSSA });

			return startSSA;
		}

	};

}