//=============================================================================
// ■ VMtest.cxx
//-----------------------------------------------------------------------------
//   标准的虚拟机环境，载入字节码并运行。
//   扩展名用了.cxx，意思是“× 不要自动编译”。
//=============================================================================

#include "ASM76.hpp"
using namespace ASM76;

static uint32_t test_bios_call(uint8_t* input) {
	printf("TEST BIOS CALL: %x\n", *((uint32_t*) input));
	return 0x89ABCDEF;
}

static uint32_t bios_call_putc(uint8_t* input) {
	putchar(*((char*) input));
	return 0x0;
}

static uint32_t bios_call_puts(uint8_t* input) {
	puts((char*) input);
	return 0x0;
}

static uint32_t bios_call_putb(uint8_t* input) {
	printf("%d\n", *((char*) input));
	return 0x0;
}

static uint32_t bios_call_puti(uint8_t* input) {
	printf("%d\n", *((int*) input));
	return 0x0;
}

static uint32_t bios_call_putl(uint8_t* input) {
	printf("%ld\n", *((long*) input));
	return 0x0;
}

BIOS* load_bios() {
	BIOS* b = new BIOS(7);
	b->function_table[1] = &test_bios_call;
	b->function_table[2] = &bios_call_putc;
	b->function_table[3] = &bios_call_puts;
	b->function_table[4] = &bios_call_putb;
	b->function_table[5] = &bios_call_puti;
	b->function_table[6] = &bios_call_putl;

	return b;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		init();

		Program p;
		const char* file_ext = strrchr(argv[1], '.') + 1;
		if ((*file_ext == 'A' || *file_ext == 'a')
			&& (file_ext[1] == 'S' || file_ext[1] == 's')) {
			char* src;
			if (slurp(argv[1], &src) < 0) {
				fprintf(stderr, "Error reading file %s", argv[1]);
				exit(2);
			}
			Assembler a(src);
			a.scan();
			p = a.assemble();
		} else {
			p = ObjectCode::read_file(argv[1]);
		}
		VM v(p);
		v.firmware = load_bios();
		v.execute(false);

		VMterminate();
	} else {
		puts("VM exec. VM/76 with BIOS version 'Still in Alpha'");
		puts("Usage: $ ./VMexec [obj_code]");
	}

	return 0;
}
