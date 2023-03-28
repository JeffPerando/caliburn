
#pragma once

#include "ast.h"

namespace caliburn
{
	struct IfStatement : public Statement
	{
		cllr::SSA id;
		Value* condition = nullptr;
		ScopeStatement* innerIf = nullptr;
		ScopeStatement* innerElse = nullptr;
		cllr::SSA postBranchID;

		IfStatement(Statement* parent) : Statement(StatementType::IF, parent) {}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			id = codeAsm.createSSA(cllr::Opcode::LABEL);

			innerIf->getSSAs(codeAsm);

			if (innerElse)
			{
				innerElse->getSSAs(codeAsm);

			}

			postBranchID = codeAsm.createSSA(cllr::Opcode::LABEL);

		}

		virtual void declSymbols(SymbolTable& table) override
		{
			innerIf->declSymbols(table);

			if (innerElse)
			{
				innerElse->declSymbols(table);

			}

		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			innerIf->resolveSymbols(table);

			if (innerElse)
			{
				innerElse->resolveSymbols(table);
			}


		}

		void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			condition->emitDeclCLLR(codeAsm);

			auto v = condition->emitLoadCLLR(codeAsm);

			codeAsm.push(id, cllr::Opcode::JUMP_COND, { v, innerIf->id, innerElse ? innerElse->id : 0 });

			innerIf->emitDeclCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::JUMP, { postBranchID });

			if (innerElse)
			{
				innerElse->emitDeclCLLR(codeAsm);

			}

			codeAsm.push(postBranchID, cllr::Opcode::LABEL, {});

		}

	};

	struct ForStatement : public Statement
	{
		Statement* preLoop = nullptr;
		Value* cond = nullptr;
		Statement* postLoop = nullptr;

		ScopeStatement* loop = nullptr;

		ForStatement(Statement* parent) : Statement(StatementType::FOR, parent) {}

		void getSSAs(cllr::Assembler& codeAsm) override
		{

		}

		void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{

		}

		/*
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
		*/

	};

	struct WhileStatement : public Statement
	{
		Value* condition = nullptr;
		ScopeStatement* loop = nullptr;

		cllr::SSA start, exit, cont;

		WhileStatement(Statement* parent) : Statement(StatementType::WHILE, parent) {}

		void getSSAs(cllr::Assembler& codeAsm) override
		{
			start = codeAsm.createSSA(cllr::Opcode::LABEL);
			exit = codeAsm.createSSA(cllr::Opcode::LABEL);
			cont = codeAsm.createSSA(cllr::Opcode::LABEL);

			loop->getSSAs(codeAsm);

		}

		void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			/*
			Until CLLR is more fleshed out, we're just going to do the SPIR-V route of putting jumps before labels
			*/
			codeAsm.push(0, cllr::Opcode::JUMP, {start, 0, 0});
			codeAsm.push(start, cllr::Opcode::LABEL, {});
			codeAsm.push(0, cllr::Opcode::LOOP, {exit, cont, loop->id});

			auto loopVal = condition->emitLoadCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::JUMP_COND, {loopVal, loop->id, exit});

			codeAsm.setLoop(cont, exit);
			loop->emitDeclCLLR(codeAsm);
			codeAsm.exitLoop();

			codeAsm.push(0, cllr::Opcode::JUMP, { cont, 0, 0 });

			codeAsm.push(cont, cllr::Opcode::LABEL, {});
			codeAsm.push(0, cllr::Opcode::JUMP, { start, 0, 0 });
			
			codeAsm.push(exit, cllr::Opcode::LABEL, {});

		}

		/*
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
		*/
	};

	struct DoWhileStatement : public Statement
	{
		Value* cond = nullptr;
		Statement* loop = nullptr;

		DoWhileStatement(Statement* parent) : Statement(StatementType::DOWHILE, parent) {}

		cllr::SSA toCLLR(cllr::Assembler& codeAsm) override
		{

		}

		/*
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
		*/

	};

}