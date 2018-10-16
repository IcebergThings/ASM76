#==============================================================================
# ■ ASM76 - msw.makefile
#==============================================================================

TARGET = ASM76.dll

MAKE76_ROOT?=.
include $(MAKE76_ROOT)/inc.makefile
include $(MAKE76_ROOT)/routine.makefile

LDFLAGS += -shared -Wl,--export-all-symbols -flto=thin
CXXFLAGS += -I.. -Ofast

test: VMtest.cxx $(TARGET)
	$(CXX) $^ -o VMtest.exe $(CXXFLAGS) $(TARGET)
	VMtest.exe

VMenv: VMc.cxx VMexec.cxx $(TARGET)
	$(CXX) VMc.cxx -o VMc.exe $(CXXFLAGS) $(TARGET)
	$(CXX) VMexec.cxx -o VMexec.exe $(CXXFLAGS) $(TARGET)

.PHONY: test
