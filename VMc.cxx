//=============================================================================
// ■ VMc.cxx
//-----------------------------------------------------------------------------
//   VM编译器，编译到字节码。
//   扩展名用了.cxx，意思是“× 不要自动编译”。
//=============================================================================

#include "ASM76.hpp"
using namespace ASM76;

int main(int argc, char** argv) {
	if (argc > 2) {
		if (strcmp(argv[1], "-d") == 0) {
			printf("Disassembling %s\n", argv[2]);

			Disassembler d(ObjectCode::read_file(argv[2]));
			char* s2 = d.disassemble();
			puts(s2);
		}
	} else if (argc > 1) {
		char* src_file = argv[1];

		char* src = NULL;
		size_t file_size = slurp(src_file, &src);
		if (file_size < 0) {
			perror("Src read failed");
			return 1;
		}

		printf("Compiling %s, src file size %ld:\n%s\n", src_file, file_size, src);

		Assembler a(src);
		a.scan();
		Program p = a.assemble();

		char* obj_file = new char[strlen(src_file) + 4];
		sprintf(obj_file, "%s.obj", src_file);
		ObjectCode::write_file(obj_file, p);
	} else {
		puts("VM compiler. VM/76 with BIOS version 'Still in Alpha'");
		puts("Usage: $./VMc [src]");
	}

	return 0;
}
