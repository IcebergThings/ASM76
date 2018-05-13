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

BIOS* load_bios() {
	BIOS b(5);
	b.function_table[1] = &test_bios_call;

	return &b;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		init();

		Program p = ObjectCode::read_file(argv[1]);
		VM v(p);
		v.firmware = load_bios();
		v.execute(true);

		VMterminate();
	} else {
		printf("VM exec. VM/76 with BIOS version 'Still in Alpha'\nUsage: $./VMexec [obj_code]");
	}

	return 0;
}
