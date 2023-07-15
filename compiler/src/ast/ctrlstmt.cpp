
#include "ast/ctrlstmt.h"

using namespace caliburn;

cllr::SSA IfStatement::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	auto cond = condition->emitValueCLLR(table, codeAsm);

	//TODO type check
	auto cID = cond.value;

	auto ifLabel = codeAsm.createSSA(cllr::Opcode::LABEL);
	auto elseLabel = codeAsm.createSSA(cllr::Opcode::LABEL);
	auto postLabel = codeAsm.createSSA(cllr::Opcode::LABEL);

	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP_COND, {}, { cID, ifLabel, innerElse ? elseLabel : postLabel }));

	innerIf->emitDeclCLLR(table, codeAsm);

	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { postLabel }));

	if (innerElse != nullptr)
	{
		innerElse->emitDeclCLLR(table, codeAsm);

	}

	codeAsm.push(cllr::Instruction(postLabel, cllr::Opcode::LABEL));

	return ifLabel;
}

cllr::SSA ForRangeStatement::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	return 0;
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

cllr::SSA WhileStatement::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	/*
	Until CLLR is more fleshed out, we're just going to do the SPIR-V route of putting jumps before labels
	*/

	auto start = codeAsm.createSSA(cllr::Opcode::LABEL);
	auto cont = codeAsm.createSSA(cllr::Opcode::LABEL);
	auto loopLabel = codeAsm.createSSA(cllr::Opcode::LABEL);
	auto exit = codeAsm.createSSA(cllr::Opcode::LABEL);

	codeAsm.pushAll({
		cllr::Instruction(cllr::Opcode::JUMP, {}, { doWhile ? loopLabel : start }),
		cllr::Instruction(start, cllr::Opcode::LABEL),
		cllr::Instruction(cllr::Opcode::LOOP, {}, { exit, cont, loopLabel }),
	});

	auto cond = condition->emitValueCLLR(table, codeAsm);

	//TODO type check
	auto cID = cond.value;

	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP_COND, {}, { cID, loopLabel, exit }));

	codeAsm.setLoop(cont, exit);
	codeAsm.push(cllr::Instruction(loopLabel, cllr::Opcode::LABEL));
	loop->emitDeclCLLR(table, codeAsm);
	codeAsm.exitLoop();

	codeAsm.pushAll({
		cllr::Instruction(cllr::Opcode::JUMP, {}, { cont }),
		cllr::Instruction(cont, cllr::Opcode::LABEL),
		cllr::Instruction(cllr::Opcode::JUMP, {}, { start }),
		cllr::Instruction(exit, cllr::Opcode::LABEL)
	});

	return start;
}
