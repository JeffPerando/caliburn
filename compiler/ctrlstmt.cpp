
#include "ctrlstmt.h"

using namespace caliburn;

void IfStatement::emitDeclCLLR(ref<cllr::Assembler> codeAsm)
{
	auto cID = condition->emitValueCLLR(codeAsm);

	codeAsm.push(id, cllr::Opcode::JUMP_COND, {}, { cID, innerIf->id, innerElse ? innerElse->id : 0 });

	innerIf->emitDeclCLLR(codeAsm);

	codeAsm.push(0, cllr::Opcode::JUMP, {}, { postBranchID });

	if (innerElse)
	{
		innerElse->emitDeclCLLR(codeAsm);

	}

	codeAsm.push(postBranchID, cllr::Opcode::LABEL, {}, {});

}

void ForRangeStatement::emitDeclCLLR(cllr::Assembler& codeAsm)
{
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
}

void WhileStatement::emitDeclCLLR(cllr::Assembler& codeAsm)
{
	/*
	Until CLLR is more fleshed out, we're just going to do the SPIR-V route of putting jumps before labels
	*/
	codeAsm.push(0, cllr::Opcode::JUMP, {}, { doWhile ? loop->id : start });
	codeAsm.push(start, cllr::Opcode::LABEL, {}, {});
	codeAsm.push(0, cllr::Opcode::LOOP, {}, { exit, cont, loop->id });

	auto cID = condition->emitValueCLLR(codeAsm);

	codeAsm.push(0, cllr::Opcode::JUMP_COND, {}, { cID, loop->id, exit });

	codeAsm.setLoop(cont, exit);
	loop->emitDeclCLLR(codeAsm);
	codeAsm.exitLoop();

	codeAsm.push(0, cllr::Opcode::JUMP, {}, { cont });

	codeAsm.push(cont, cllr::Opcode::LABEL, {}, {});
	codeAsm.push(0, cllr::Opcode::JUMP, {}, { start });

	codeAsm.push(exit, cllr::Opcode::LABEL, {}, {});

}
