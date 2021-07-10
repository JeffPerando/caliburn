
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

			//this assumes the else branch pointer is either null or valid
			//which imo is a safe assumption, but lord knows there's certain
			//people who think null is evil.
			//not gonna tirade about pointers, but let me ask you this:
			//if I have a pointer that's some invalid, magic number, how am
			//I going to validate it without knowing the magic number? And how
			//would others know to use THAT magic number? how will we prevent
			//other compilers from using other, equally arbitrary magic num-- ah
			//ffs this is a tirade, nvm... please just use null or a valid pointer
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
		ValueStatement* cond = nullptr;
		Statement* incr = nullptr;
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
			codeAsm->pushAll({
				spirv::OpBranch(), startSSA,
				spirv::OpLabel(), startSSA,
				//for
				spirv::OpLoopMerge(0), mergeSSA, contSSA, 0, //TODO base flag off optimize level
				//workaround for OpLoopMerge needing a branch op after
				spirv::OpBranch(), bodySSA,
				spirv::OpLabel(), bodySSA });
			
			//(int i; i < j;
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
			incr->toSPIRV(codeAsm);

			codeAsm->pushAll({
				//loop back
				spirv::OpBranch(), startSSA,
				//merge point
				spirv::OpLabel(), mergeSSA
				});
			
		}

	};

}