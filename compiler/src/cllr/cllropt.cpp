
#include "cllr/cllropt.h"

using namespace caliburn;

void caliburn::cllr::Optimizer::optimize(out<cllr::Assembler> cllrAsm)
{
	if (settings->o == OptimizeLevel::DEBUG)
	{
		return;
	}

	//RUN THIS LAST.
	cllrAsm.flatten();

}
