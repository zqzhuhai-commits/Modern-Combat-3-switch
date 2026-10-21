# Independent AArch32 runtime port of Modern Combat 3: Fallen Nation.
TARGET := mc3_nx
BUILD := build
SOURCES := nx-port-source
PORT_NPDM_PROGRAM_ID := 0x0100F5344D433300
PORT_NPDM_NAME := mc3
PORT_NPDM_VERSION := 0.1.0
PORT_NPDM_MAIN_STACK := 0x800000
DCR_GL_MESA := 1
PORT_MESA := 20
PORTLIBS ?= $(CURDIR)/portlibs32
include runtime/runtime.mk
# MC3 imports a pure GLES2/EXT set; no GLES1 fixed-function needed.
LIBS += -L$(PORTLIBS)/lib -lGLESv2

$(BUILD)/$(TARGET).nso: $(TARGET).elf nx-port-tools/elf32_to_nso.py | $(BUILD)
	python3 nx-port-tools/elf32_to_nso.py $< $@
