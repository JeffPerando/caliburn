
#pragma once

#include "langcore.h"

#include "ast/fn.h"

#include "cllr/cllrtype.h"

namespace caliburn
{
	enum class TypeCategory : uint32_t
	{
		VOID,
		FLOAT,
		INT,
		VECTOR,
		MATRIX,
		ARRAY,
		STRUCT,
		BOOLEAN,
		POINTER,
		TEXTURE
		//TUPLE
		//STRING

	};

	struct BaseType
	{
		const TypeCategory category;
		const std::string canonName;

		FunctionGroup ctors;
		sptr<Method> dtor = nullptr;

		BaseType(TypeCategory c, in<std::string> n) :
			category(c), canonName(n) {}

		virtual ~BaseType() = default;

		bool operator!=(in<BaseType> rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(in<BaseType> rhs) const
		{
			return canonName == rhs.canonName;
		}

		virtual sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

	protected:
		virtual void initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const {}

	};

	struct TypeArray : BaseType
	{
		const GenericSignature sig = GenericSignature(std::vector{
				GenericName(GenericSymType::TYPE, "T"),
				GenericName(GenericSymType::CONST, "N")
			});

		TypeArray() : BaseType(TypeCategory::ARRAY, "array") {}
		virtual ~TypeArray() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeBool : BaseType
	{
		TypeBool() : BaseType(TypeCategory::BOOLEAN, "bool") {}
		virtual ~TypeBool() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
		}

	};

	struct TypeFloat : BaseType
	{
		const uint32_t width;

		TypeFloat(uint32_t bits) : BaseType(TypeCategory::FLOAT, "fp" + std::to_string(bits)), width(bits) {}
		virtual ~TypeFloat() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_FLOAT, { width }));

			initLowImpl(impl, gArgs, table, codeAsm);

			return impl;
		}

		void initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct TypeInt : BaseType
	{
		const uint32_t width;
		const bool isSigned;

		TypeInt(uint32_t bits, bool sign) : BaseType(TypeCategory::INT, (isSigned ? "int" : "uint") + std::to_string(bits)), width(bits), isSigned(sign) {}
		virtual ~TypeInt() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			auto const typeOp = (isSigned ? cllr::Opcode::TYPE_INT_SIGN : cllr::Opcode::TYPE_INT_UNSIGN);

			return codeAsm.pushType(cllr::Instruction(typeOp, { width }));
		}
		
		void initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct TypeStruct : BaseType
	{
		const uptr<GenericSignature> genSig;

		std::map<std::string_view, sptr<ParsedVar>> members;
		std::vector<uptr<ParsedFn>> memberFns;
		GenArgMap<cllr::LowType> variants;

		TypeStruct(std::string_view name, out<uptr<GenericSignature>> sig, in<std::map<std::string_view, sptr<ParsedVar>>> members, out<std::vector<uptr<ParsedFn>>> fns)
			: BaseType(TypeCategory::STRUCT, std::string(name)), genSig(std::move(sig)), members(members), memberFns(std::move(fns)) {}

		virtual ~TypeStruct() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeTexture : BaseType
	{
	private:
		const TextureKind kind;

		const GenericSignature genSig = GenericSignature({
			GenericName{GenericSymType::TYPE, "Pixel", new_sptr<ParsedType>("vec4")}
			});

	public:
		TypeTexture(TextureKind tk) : BaseType(TypeCategory::TEXTURE, std::string("tex").append(TEX_TYPES.at(tk))), kind(tk) {}
		virtual ~TypeTexture() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		void initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct TypeVector : BaseType
	{
	private:
		GenArgMap<cllr::LowType> variants;
	public:
		const uint32_t elements;
		const GenericSignature genSig = GenericSignature({
			GenericName(GenericSymType::TYPE, "T", GenericResult(new_sptr<ParsedType>("fp32")))
			});

		TypeVector(uint32_t vecElements) :
			BaseType(TypeCategory::VECTOR, "vec" + std::to_string(vecElements)),
			elements(vecElements)
		{}

		virtual ~TypeVector() = default;

		virtual sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		void initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct TypeVoid : BaseType
	{
		TypeVoid() : BaseType(TypeCategory::VOID, "void") {}
		virtual ~TypeVoid() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VOID));
		}

	};

}