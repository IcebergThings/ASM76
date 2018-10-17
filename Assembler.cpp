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
	// ● Tokenize
	//-------------------------------------------------------------------------
	std::vector<char*> Assembler::tokenize(const char* prg) {
		std::vector<char*> token_list = {};
		std::string current_token = "";

		bool comment_mode = false;
		bool string_mode = false;
		bool string_escape = false;

		while (*prg != '\0') {
			char c = *prg;
			if (comment_mode) {
				// Wait for new line
				if (c == '\n') comment_mode = false;
			} else if (string_mode) {
				if (string_escape) {
					// Now escape only, no \n stuff
					current_token.push_back(c);
					string_escape = false;
				} else if (c == '\\') {
					// Start escape
					string_escape = true;
				} else {
					if (c == '"') string_mode = false;
					current_token.push_back(c);
				}
			} else {
				if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '#' || c == '"') {
					// Previous token ends, new token starts
					// If current_token is empty, do nothing.
					if (current_token.length()) {
						size_t buf_size = sizeof(char) * (current_token.length() + 1);
						char* token_cstr = (char*) malloc(buf_size);
						strcpy(token_cstr, current_token.c_str());
						token_list.push_back(token_cstr);
						current_token.clear();
					}
					if (c == '#') comment_mode = true;
					if (c == '"') {
						string_mode = true;
						current_token.push_back(c);
					}
				} else {
					// Push into current token as it's not whitespace or any seperator
					current_token.push_back(c);
				}
			}
			prg++;
		}

		return token_list;
	}
	//-------------------------------------------------------------------------
	// ● Token类型判断
	//-------------------------------------------------------------------------
	bool Assembler::is_string_literal(const char* s) {
		if (strlen(s) < 2) return false;
		return s[0] == '"' && s[strlen(s) - 1] == '"';
	}

	bool Assembler::is_number_literal(const char* s) {
		char* _s = (char*) s;
		if (strlen(s) > 2) {
			if (s[0] == '0' && s[1] == 'x') {
				// Hex
				_s += 2;
				while (*_s) {
					char c = toupper(*_s);
					if ((c < '0' or c > '9') and (c < 'A' or c > 'F')) return false;
					_s ++;
				}
				return true;
			}
		}
		// Digits
		while (*_s) {
			if (*_s < '0' or *_s > '9') return false;
			_s ++;
		}
		return true;
	}

	bool Assembler::is_symbol_literal(const char* s) {
		if (strlen(s) < 2) return false;
		return s[0] == '[' && s[strlen(s) - 1] == ']';
	}

	bool Assembler::is_register_literal(const char* s) {
		if (strlen(s) < 2) return false;
		return s[0] == '$' && Assembler::is_number_literal(s + 1);
	}
	//-------------------------------------------------------------------------
	// ● 报错
	//-------------------------------------------------------------------------
	void Assembler::error(const char* message) {
		printf("Compile Error: %s\n", message);
		exit(1);
	}
	//-------------------------------------------------------------------------
	// ● Token -> Useful Data
	//-------------------------------------------------------------------------
	uint16_t Assembler::read_opcode(const char* s) {
		#define I(op, a, b) \
			if (strcasecmp(#op , s) == 0) { return op; }
		#include "instructions.hpp"
		return 0;
	}
	
	//-------------------------------------------------------------------------
	// ● 编译
	//-------------------------------------------------------------------------
	Program Assembler::assemble() {
		std::vector<char*> token_list = Assembler::tokenize(original_prg);
		int token_list_length = token_list.size();
		
		uint8_t* output = (uint8_t*) malloc(0xFF);
		int output_capacity = 0xFF;
		int output_size = 0;
		
		int current_state = Inactive;

		#define CHECK_EXPAND(s) \
			output_size += s; \
			if (s < 0) error("Can't expand negative size."); \
			if (output_size > output_capacity) { \
				output_capacity = 1; \
				while (output_capacity < output_size) output_capacity *= 2; \
				(void)realloc((void*) output, output_capacity * sizeof(uint8_t)); \
			}

		const char* empty_token = "\0";

		for (int index = 0; index <= token_list_length; index++) {
			char* token;
			if (index == token_list_length) {
				if (current_state == Inactive) break;
				printf("Reached end. Append empty token.\n");
				token = (char*) empty_token;
			} else {
				token = token_list[index];
			}
			
			if (current_state == Inactive) {
				// Special handling for Emit Data & Tag Declare
				if (strcasecmp(token, "RAWD") == 0 || strcasecmp(token, "FILL") == 0) {
					current_state = EmitData;
					printf("Start emitting data\n");
				} else if (is_symbol_literal(token)) {
					printf("Symbol %s declared at 0x%x\n", token, output_size);
				} else {
					uint16_t opcode = Assembler::read_opcode(token);
					uint32_t output_ptr = output_size;
					printf("0x%x : %s<%x>", output_size, token, opcode);
					#define TNUL
					#define TIMM \
						CHECK_EXPAND(sizeof(uint32_t)) \
						index++; if (index >= token_list_length) error("Unexpected EOF"); \
						printf(" %s ", token_list[index]);
					#define TADD \
						CHECK_EXPAND(sizeof(uint32_t)) \
						index++; if (index >= token_list_length) error("Unexpected EOF"); \
						printf(" %s ", token_list[index]);
					#define TREG \
						CHECK_EXPAND(sizeof(uint32_t)) \
						index++; if (index >= token_list_length) error("Unexpected EOF"); \
						printf(" %s ", token_list[index]);
					#define I(op, Ta, Tb) \
						case op: \
							CHECK_EXPAND(sizeof(uint16_t)) \
							*((uint16_t*) (output + output_ptr)) = op; \
							Ta \
							Tb \
							break;

					switch(opcode){
						#include "instructions.hpp"
						default:
							printf("Can't under stand token %s", token);
					}
					printf("\n");
				}
			} else if (current_state == EmitData) {
				// If receiving Hex/Digits or String, fill the output
				// If receiving other things, return to normal state
				bool is_num = is_number_literal(token);
				if (is_num || is_string_literal(token)) {
					int output_ptr = output_size;
					printf("0x%x : %s\n", output_ptr, token);
					if (is_num) {
						// Assume all hex are two bytes
						// Assume all dec digits are uint32_t
						if (strlen(token) > 2 && token[1] == 'x') {
							CHECK_EXPAND(2)
							*((uint16_t*) (output + output_ptr)) = std::stoi(token, nullptr, 0);
						} else {
							CHECK_EXPAND(4)
							*((uint32_t*) (output + output_ptr)) = std::stoi(token, nullptr, 0);
						}
					} else {
						CHECK_EXPAND(strlen(token) - 2)
						for (uint32_t i = 1; i < strlen(token) - 1; i++) {
							*((char*) (output + output_ptr + i - 1)) = token[i];
						}
					}
				} else {
					current_state = Inactive;
					index --; // Rewind one token
					printf("Token %s is not data. End emitting data\n", token);
				}
			} else {
				error("Compiler can't be in a WTF emit state");
			}
		}

		// Clean memory
		for (char* token : token_list) {
			free(token);
		}
		token_list.clear();

		Instruct* output_raw = (Instruct*) malloc(output_size * sizeof(uint8_t));
		memcpy(output_raw, output, output_size * sizeof(uint8_t));

		printf("Compile ends. Program size: 0x%x\n", output_size);

		Program p = {
			.size = output_size * sizeof(uint8_t),
			.instruct = (Instruct*) output_raw
		};
		return p;
	}
}
