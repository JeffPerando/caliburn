
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct IfStatement : public Statement
	{
		ValueStatement* condition = nullptr;
		Statement* ifBranch = nullptr;
		Statement* elseBranch = nullptr;

		IfStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			uint32_t condSSA = condition->toSPIRV(codeAsm);
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

			ifBranch->toSPIRV(codeAsm);
			
			codeAsm->push(spirv::OpBranch());
			codeAsm->push(endSSA);

			if (elseBranch)
			{
				codeAsm->push(spirv::OpLabel());
				codeAsm->push(elseSSA);

				elseBranch->toSPIRV(codeAsm);

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

		ForStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
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

			//(int i = 0;
			preLoop->toSPIRV(codeAsm);

			codeAsm->pushAll({
				spirv::OpLoopMerge(0), mergeSSA, contSSA, 0, //TODO base flag off optimize level
				//workaround for OpLoopMerge needing a branch op after
				spirv::OpBranch(), bodySSA,
				spirv::OpLabel(), bodySSA });
			
			//i < j;
			uint32_t condSSA = cond->toSPIRV(codeAsm);

			//branch
			codeAsm->pushAll({
				spirv::OpBranchConditional(0), condSSA, loopSSA, mergeSSA,
				//{
				spirv::OpLabel(), loopSSA });
			
			loop->toSPIRV(codeAsm);

			codeAsm->pushAll({
				spirv::OpBranch(), contSSA,
				spirv::OpLabel(), contSSA });
			//}
			//i++)
			postLoop->toSPIRV(codeAsm);

			codeAsm->pushAll({
				//loop back
				spirv::OpBranch(), startSSA,
				//merge point
				spirv::OpLabel(), mergeSSA
				});
			
		}

	};

	struct WhileStatement : public Statement
	{
		ValueStatement* cond = nullptr;
		Statement* loop = nullptr;

		WhileStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
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
			uint32_t condSSA = cond->toSPIRV(codeAsm);

			//branch
			codeAsm->pushAll({
				spirv::OpBranchConditional(0), condSSA, loopSSA, mergeSSA,
				//{
				spirv::OpLabel(), loopSSA });

			loop->toSPIRV(codeAsm);

			codeAsm->pushAll({
				spirv::OpBranch(), contSSA,
				spirv::OpLabel(), contSSA });
			//}
			
			codeAsm->pushAll({
				//loop back
				spirv::OpBranch(), startSSA,
				//merge point
				spirv::OpLabel(), mergeSSA
				});

		}

	};

	struct ReturnStatement : public Statement
	{
		ValueStatement* val = nullptr;
		
		ReturnStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			if (val)
			{
				uint32_t retval = val->toSPIRV(codeAsm);

				//codeAsm->pushVarSetter(CALIBURN_RETURN_VAR, retval);
				codeAsm->pushAll({ spirv::OpReturnValue(), retval });

			}
			else
			{
				codeAsm->push(spirv::OpReturn());
			}
			
			return 0;
		}
	};

}