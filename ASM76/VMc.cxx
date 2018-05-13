//=============================================================================
// ■ VMc.cxx
//-----------------------------------------------------------------------------
//   VM编译器，编译到字节码。
//   扩展名用了.cxx，意思是“× 不要自动编译”。
//=============================================================================

#include "ASM76.hpp"
using namespace ASM76;

long slurp(char const* path, char **buf) {
	FILE  *fp;
	size_t fsz;
	long   off_end;
	int    rc;

	/* Open the file */
	fp = fopen(path, "rb");
	if( NULL == fp ) {
		return -1L;
	}

	rc = fseek(fp, 0L, SEEK_END);
	if( 0 != rc ) {
		return -1L;
	}

	if( 0 > (off_end = ftell(fp)) ) {
		return -1L;
	}
	fsz = (size_t)off_end;

	*buf = (char*) malloc(fsz);
	if( NULL == *buf ) {
		return -1L;
	}

	rewind(fp);

	if( fsz != fread(*buf, 1, fsz, fp) ) {
		free(*buf);
		return -1L;
	}

	if( EOF == fclose(fp) ) {
		free(*buf);
		return -1L;
	}

	return (long)fsz;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		char* src_file = argv[1];

		char* src = NULL;
		long file_size = slurp(src_file, &src);
		if( file_size < 0L ) {
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
		printf("VM compiler. VM/76 with BIOS version 'Still in Alpha'\nUsage: $./VMc [src]\n");
	}

	return 0;
}
