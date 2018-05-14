#==============================================================================
# ■ ASM76 - msw.makefile
#==============================================================================

TARGET = ASM76.dll

include ../inc.makefile
include ../routine.makefile

LDFLAGS += -shared -Wl,--export-all-symbols
CXXFLAGS += -I..

test: VMtest.cxx $(TARGET)
	$(CXX) $^ -o VMtest.exe $(CXXFLAGS) $(TARGET)
	VMtest.exe

VMenv: VMc.cxx VMexec.cxx $(TARGET)
	$(CXX) VMc.cxx -o VMc.exe $(CXXFLAGS) $(TARGET)
	$(CXX) VMexec.cxx -o VMexec.exe $(CXXFLAGS) $(TARGET)

.PHONY: test
