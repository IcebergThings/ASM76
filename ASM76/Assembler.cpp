//=============================================================================
// ■ Assembler.cpp
//-----------------------------------------------------------------------------
//   ASM76汇编器。
//=============================================================================

#include "ASM76.hpp"

namespace ASM76 {
	//-------------------------------------------------------------------------
	// ● 构造
	//-------------------------------------------------------------------------
	Assembler::Assembler(const char* program) {
		prg = original_prg = program;

		memset(RegVars, 0x0, sizeof(RegVars));
	}
	//-------------------------------------------------------------------------
	// ● 第一遍扫描（扫描标签）
	//-------------------------------------------------------------------------
	void Assembler::scan() {
		uint32_t size = 0x0;
		while (prg && *prg) switch (*prg) {
		case '[': {
			prg++;
			char* tagname = new char[MAX_TAG_NAME_SIZE];
			copy_tagname(tagname);
			Tag tag;
			tag.name = tagname;
			tag.pointer = size;
			printf("Tag '%s': 0x%x\n", tag.name, tag.pointer);
			tags.push(tag);
			break;
		}
		case '\n':
			prg++;
			break;
		case '{':
		case '#':
			prg = strchr(prg, '\n') + 1;
			ensure_prg();
			break;
		case '*':
			prg++;
			while (*prg != '\n' && *prg) {
				size++;
				prg++;
			}
			size++; // reserve space for 0x00
			skip('\n');
			break;
		default:
			size += sizeof(Instruct);
			prg = strchr(prg, '\n') + 1;
			ensure_prg();
		}
		prg = original_prg;
	}
	//-------------------------------------------------------------------------
	// ● 第二遍扫描（汇编）
	//-------------------------------------------------------------------------
	Program Assembler::assemble() {
		V::Vector<uint8_t, false> instructs(120);
		while (prg && *prg) switch (*prg) {
		case '#':
		case '[':
			prg = strchr(prg, '\n') + 1;
			break;
		case '{':
			// 读取宏操作
			prg++;
			char macro_operation[13];
			copy_opcode(macro_operation);

			if (strcmp(macro_operation, "AllocRegVar") == 0) {
				char identifier[13];
				copy_varname(identifier);
				alloc_reg_var(identifier, read_immediate_u32());
			} else if (strcmp(macro_operation, "FreeRegVar") == 0) {
				char identifier[13];
				copy_varname(identifier);
				free_reg_var(identifier);
			}

			prg = strchr(prg, '\n') + 1;
			ensure_prg();

			break;
		case '\n':
			prg++;
			break;
		case '*':
			prg++;
			while (*prg && *prg != '\n') {
				instructs.push(*prg);
				prg++;
			}
			instructs.push(0x00);
			skip('\n');
			break;
		default:
			Instruct i;
			// 读取opcode
			char opcode[13];
			copy_opcode(opcode);
			i.opcode = parse_opcode(opcode);
			// 读取参数
			read_parameters(&i);
			// 读取换行
			skip('\n');
			// 将Instruct转换为字节数组保存
			auto b = (uint8_t*) &i;
			for (size_t i = 0; i < sizeof(Instruct); i++) {
				instructs.push(b[i]);
			}
		}
		Program r;
		r.instruct = (Instruct*) instructs.start;
		r.size = instructs.size();
		return r;
	}
	//-------------------------------------------------------------------------
	// ● 报错！
	//-------------------------------------------------------------------------
	void Assembler::error(const char* message) {
		printf("Error: %s\nAssembly:\n", message);
		const char* p = prg;
		// find spp
		// spp = start of printed program
		while (p > original_prg && p[-1] != '\n') p--;
		const char* spp = p;
		if (p) {
			while (!check(*p, "\n")) {
				char c = *p++;
				if (c == '\t') c = ' ';
				putchar(c);
			}
			putchar('\n');
		} else {
			puts("(no source)");
		}
		// print the caret
		int loc = prg - spp;
		while (loc--) putchar(' ');
		puts("^");
		// exit gracefully
		abort();
	}
	//-------------------------------------------------------------------------
	// ● 检查prg是不是跑飞了
	//-------------------------------------------------------------------------
	void Assembler::ensure_prg() {
		if (prg < original_prg) {
			prg = NULL;
			error("My caret disappeared while assembling. "
				"Probably you are missing a newline at the end of your file.");
		}
	}
	//-------------------------------------------------------------------------
	// ● 是否为某些字符中的一个？
	//   若c为零，则必然返回true，因为s是由零结尾的。
	//-------------------------------------------------------------------------
	bool Assembler::check(char c, const char* s) {
		return strchr(s, (unsigned char) c) != NULL;
	}
	//-------------------------------------------------------------------------
	// ● 跳过某一字符
	//-------------------------------------------------------------------------
	void Assembler::skip(char c) {
		if (*prg != c) {
			char msg[20];
			switch (c) {
			case '\n':
				strcpy(msg, "expected newline");
				break;
			case ' ':
				strcpy(msg, "expected space");
				break;
			default:
				strcpy(msg, "expected '\a'");
				*strchr(msg, '\a') = c;
			}
			error(msg);
		}
		prg++;
	}
	//-------------------------------------------------------------------------
	// ● 跳过某些字符组成的串
	//-------------------------------------------------------------------------
	void Assembler::skip(const char* s, const char* error_msg) {
		size_t len = strspn(prg, s);
		if (!len) error(error_msg);
		prg += len;
	}
	//-------------------------------------------------------------------------
	// ● 复制标签名称
	//-------------------------------------------------------------------------
	void Assembler::copy_tagname(char* buf) {
		for (size_t i = 0; i < MAX_TAG_NAME_SIZE; i++) {
			if (prg[i] == '\n') {
				error("tag name contains newline");
				return;
			}
			if (prg[i] == ']') {
				memcpy(buf, prg, i);
				buf[i] = 0;
				prg += i + 1;
				skip('\n');
				return;
			}
		}
		error("tag name too long");
	}
	//-------------------------------------------------------------------------
	// ● 复制opcode
	//-------------------------------------------------------------------------
	void Assembler::copy_opcode(char* buf) {
		for (size_t i = 0; i < 12; i++) {
			if (check(prg[i], " \t\v\n")) {
				memcpy(buf, prg, i);
				buf[i] = 0;
				prg += i;
				return;
			}
		}
		error("opcode too long");
	}
	//-------------------------------------------------------------------------
	// ● 复制变量名称
	//-------------------------------------------------------------------------
	void Assembler::copy_varname(char* buf) {
		skip(" \t\v\n", "expected whitespace");
		for (size_t i = 0; i < MAX_TAG_NAME_SIZE; i++) {
			if (check(prg[i], "} \t\v\n")) {
				memcpy(buf, prg, i);
				buf[i] = 0;
				prg += i;
				return;
			}
		}
		error("variable name too long");
	}
	//-------------------------------------------------------------------------
	// ● opcode: char[] → enum InstructionOpcode
	//-------------------------------------------------------------------------
	enum InstructionOpcode Assembler::parse_opcode(const char* str) {
		#define I(x, ta, tb) if (strcmp(str, #x) == 0) return x;
		#include "instructions.hpp"
		// RAW data instruction (e.g. RAWD) with a virtual opcode 512
		// This instruction takes in 10 bytes, exactly,
		// in the format of RAWD 0xFFFF 0xFFFF 0xFFFF 0xFFFF 0xFFFF.
		if (strcmp(str, "RAWD") == 0) return RAWD;

		error("unidentified instruction");
		return NOOP;
	}
	//-------------------------------------------------------------------------
	// ● 读取参数
	//-------------------------------------------------------------------------
	void Assembler::read_parameters(Instruct* i) {
		switch (i->opcode) {
		#define TNUL 0
		#define TIMM read_address()
		#define TADD read_address()
		#define TREG read_register()
		#define I(x, ta, tb) case x: i->a = ta; i->b = tb; break;
		#include "instructions.hpp"
		// Data instruction
		case RAWD: {
			uint16_t bits[5];
			bits[0] = read_immediate_u32() & 0xFFFF;
			bits[1] = read_immediate_u32() & 0xFFFF;
			bits[2] = read_immediate_u32() & 0xFFFF;
			bits[3] = read_immediate_u32() & 0xFFFF;
			bits[4] = read_immediate_u32() & 0xFFFF;

			i->opcode = bits[0];
			i->a = (bits[1] << 16) | bits[2];
			i->b = (bits[3] << 16) | bits[4];
			break;
		}
		}
	}
	//-------------------------------------------------------------------------
	// ● 分配寄存器变量
	//-------------------------------------------------------------------------
	void Assembler::alloc_reg_var(const char* identifier, int length) {
		// Skip $0 ~ $4
		for (int i = 4; i < 100; i++) {
			if (!RegVars[i]) {
				bool found = true;
				// Find one free, ensure the whole length is free
				for (int j = 1; j < length; j++) if (RegVars[i + j]) {
					found = false;
					break;
				}
				// Found one free space, alloc and exit
				if (found) {
					RegVar* temp_var = new RegVar;
					temp_var->identifier = new char[strlen(identifier + 1)];
					temp_var->reg = i;
					temp_var->length = length;
					strcpy(temp_var->identifier, identifier);
					for (int j = 0; j < length; j++) RegVars[i + j] = temp_var;
					printf("Register variable $%s allocated from $%d to $%d.\n", identifier, i, i + length - 1);
					return;
				}
			}
		}
		error("The registers are used up!");
	}
	//-------------------------------------------------------------------------
	// ● 释放寄存器变量
	//-------------------------------------------------------------------------
	void Assembler::free_reg_var(const char* identifier) {
		// Skip $0 ~ $4
		for (int i = 4; i < 100; i++) {
			if (RegVars[i] && RegVars[i]->identifier && strcmp(RegVars[i]->identifier, identifier) == 0) {
				RegVar* temp_var = RegVars[i];
				for (int j = 0; j < temp_var->length; j++) RegVars[i + j] = NULL;
				free(temp_var);
				printf("Register variable $%s released ($%d - $%d).\n", identifier, i, i + temp_var->length - 1);
				return;
			}
		}
		printf("Register variable with identifier %s not found.\n", identifier);
		error("Attempt to free non-exist variable");
	}
	//-------------------------------------------------------------------------
	// ● 读取立即数参数
	//-------------------------------------------------------------------------
	uint32_t Assembler::read_immediate_u32() {
		skip(" \t\v", "expected whitespace");
		if (!isdigit((unsigned char) *prg)) error("expected digit");
		long long n;
		char* end;
		if (prg[0] == '0' && prg[1] == 'x') {
			n = strtoll(prg + 2, &end, 16);
		} else {
			n = strtoll(prg, &end, 10);
		}
		if (n > UINT32_MAX) error("immediate number too large");
		if (n < 0) error("immediate number can't be negative");
		prg = (const char*) end;
		return n;
	}
	//-------------------------------------------------------------------------
	// ● 读取地址参数
	//-------------------------------------------------------------------------
	uint32_t Assembler::read_address() {
		return prg[1] == '[' ? read_address_tag() : read_immediate_u32();
	}
	//-------------------------------------------------------------------------
	// ● 读取标签参数
	//-------------------------------------------------------------------------
	uint32_t Assembler::read_address_tag() {
		skip(" \t\v", "expected whitespace");
		skip('[');
		char name[MAX_TAG_NAME_SIZE];
		for (size_t i = 0; i < MAX_TAG_NAME_SIZE + 1; i++) if (prg[i] == ']') {
			memcpy(name, prg, i);
			name[i] = 0;
			prg += i + 1;
			for (size_t i = 0; i < tags.size(); i++) {
				if (strcmp(name, tags[i].name) == 0) {
					return tags[i].pointer;
				}
			}
			printf("* Note: tag name = %s\n", name);
			error("tag name not found");
			return 0;
		}
		error("tag name too long");
		return 0;
	}
	//-------------------------------------------------------------------------
	// ● 读取寄存器参数
	//   返回寄存器编号。
	//-------------------------------------------------------------------------
	uint32_t Assembler::read_register() {
		skip(" \t\v", "expected whitespace");
		skip('$');
		if (!isdigit((unsigned char) *prg)) error("expected digit");
		int reg = atoi(prg);
		if (reg < 0) error("register no. can't be negative");
		if ((size_t) reg > VM::REGISTER_COUNT) error("register no. too large");
		while (isdigit((unsigned char) *prg)) prg++;
		return reg;
	}
}
