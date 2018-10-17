//=============================================================================
// ■ ASM76.hpp
//=============================================================================

#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <vector>
#include <string>
using namespace std;

#include "VLib/V.hpp"

#ifndef _INCLUDE_ASM76_H
#define _INCLUDE_ASM76_H

namespace ASM76 {
	//-------------------------------------------------------------------------
	// ● 指令结构的定义
	//-------------------------------------------------------------------------
	enum InstructionOpcode {
		#define I(x, a, b) x,
		#include "instructions.hpp"
		RAWD = 512,
	};
	enum InstructionOperandType {
		NONE,
		IMMEDIATE,
		ADDRESS,
		REGISTER,
	};
	struct [[gnu::packed]] Instruct {
		uint16_t opcode;
		uint32_t a;
		uint32_t b;
	};
	struct Program {
		size_t size; // should be in bytes, cleanup required
		Instruct* instruct;
	};
	//-------------------------------------------------------------------------
	// ● 全局变量
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// ● 实用函数
	//-------------------------------------------------------------------------
	void init();
	void VMterminate();
	long slurp(const char* filename, char** buf);
	//-------------------------------------------------------------------------
	// ● 汇编器
	//-------------------------------------------------------------------------
	class Assembler {
	public:
		struct Tag {
			char* name;
			uint32_t pointer;
		};
		static const size_t MAX_TAG_NAME_SIZE = 256;

		struct RegVar {
			char* identifier;
			int reg;
			int length;
		};
	private:
		const char* original_prg;
		const char* prg;
		V::Vector<Tag> tags;
		RegVar* RegVars[108];

		struct SymbolPlaceHolder {
			char* identifier;
			uint32_t* ptr;
		};

		enum EmitState {
			Inactive,
			EmitData,
			EmitInst,
		};
	public:
		Assembler(const char*);

		static std::vector<char*> tokenize(const char* prg);
		static bool is_string_literal(const char* s);
		static bool is_number_literal(const char* s);
		static bool is_symbol_literal(const char* s);
		static bool is_register_literal(const char* s);
		static uint16_t read_opcode(const char* s);

		void scan() {};
		Program assemble();
	private:
		void error(const char* message);
		void ensure_prg();
		bool check(char c, const char* s);
		void skip(char);
		void skip(const char* s, const char* error_msg);
		void skip_if(const char* s);
		void copy_opcode(char* buf);
		void copy_tagname(char* buf);
		void copy_varname(char* buf);
		enum InstructionOpcode parse_opcode(const char* str);
		void read_parameters(Instruct* i);

		void alloc_reg_var(const char* identifier, int size);
		void free_reg_var(const char* identifier);

		uint32_t read_immediate_u32();
		uint32_t read_address();
		uint32_t read_address_tag();
		uint32_t read_register();
	};
	//-------------------------------------------------------------------------
	// ● 反汇编器
	//-------------------------------------------------------------------------
	class Disassembler {
	private:
		Program prg;
	public:
		Disassembler(const Program);
		char* disassemble();
		const char* get_opcode_name(enum InstructionOpcode opcode);
		void append_number(char* line, uint32_t x);
		void append_register(char* line, uint32_t r);
		void append_address(char* line, uint32_t a);
	};
	//-------------------------------------------------------------------------
	// ● 目标代码（object code）读写
	//-------------------------------------------------------------------------
	namespace ObjectCode {
		Program read_file(const char* filename);
		bool write_file(const char* filename, Program program);
	}
	//-------------------------------------------------------------------------
	// ● BIOS类
	//-------------------------------------------------------------------------
	// Input “databuf” which is a VM76 memory address
	// Output a VM76 memory address or any uint32_t value
	// BIOS calls are locked, which is thread safe.
	typedef uint32_t (*BIOS_call)(uint8_t* d);

	class BIOS {
	public:
		BIOS_call* function_table;

		BIOS(size_t function_table_count);
		~BIOS();

		uint32_t call(int fid, uint8_t* d);
	};
	//-------------------------------------------------------------------------
	// ● Virtual Machine类
	//-------------------------------------------------------------------------
	class VM {
	public:
		static const size_t REGISTER_COUNT = 112;
	private:
		uint8_t* local_memory;
		size_t local_memory_size = 0x4000;
		Instruct* instruct_memory;
		uint8_t reg[REGISTER_COUNT];

		// For optimization purpose, let's make REG100 non-exposed (who needs that anyways)
		uint32_t _reg_100;

		// Common & special registers
		#define REG(T, P) (*((T*) &reg[P]))
		#define REG100 _reg_100
		#define REG104 REG(uint32_t, 104)
		#define REGCMP REG(uint8_t, 109)
		#define REGISF REG(uint8_t, 110)

	public:
		BIOS* firmware;

		template <class T> T* memfetch(uint32_t address) {
			return (T*) (local_memory + address);
		}

		VM(Program program);
		~VM();
		void dump_registers();
		void dump_memory();

		void execute_from(uint32_t start_pos, bool debug_process);
		void execute(bool debug_process);
		inline void execute_instruction_inline(uint16_t opcode, uint32_t a, uint32_t b);
		void execute_instruction(Instruct*);
		#define I(x, ta, tb) void execute_##x(uint32_t a, uint32_t b);
		#include "instructions.hpp"
	};
}

#endif
