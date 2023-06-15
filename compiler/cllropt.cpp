
#include "cllropt.h"

using namespace caliburn;

void cllr::optimize(OptimizeLevel o, ref<cllr::Assembler> cllrAsm)
{
	if (o == OptimizeLevel::DEBUG)
	{
		return;
	}

	//RUN THIS LAST.
	cllrAsm.flatten();

}
