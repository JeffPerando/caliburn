
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeInt : public ConcreteType
	{
	protected:
		const uint32_t intBits;
	public:
		TypeInt() : TypeInt(32, true) {}
		TypeInt(uint32_t b, bool s) :
			ConcreteType(TypeCategory::PRIMITIVE,
				(s ? "int" : "uint") + b,
				{ (s ? TypeAttrib::SIGNED : TypeAttrib::NONE) }),
			intBits(b)
		{}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		void getConvertibleTypes(std::set<ConcreteType*>* types, CaliburnAssembler* codeAsm) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, ConcreteType* rType, uint32_t rvalueSSA, ConcreteType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, ConcreteType*& endType) const override;

	};

}