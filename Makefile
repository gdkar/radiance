PROJECT = radiance
#CC = gcc
#CC = clang

# Source files
C_SRC  = $(wildcard *.c)
C_SRC += $(wildcard audio/*.c)
C_SRC += $(wildcard midi/*.c)
C_SRC += $(wildcard output/*.c)
C_SRC += $(wildcard pattern/*.c)
C_SRC += $(wildcard time/*.c)
C_SRC += $(wildcard ui/*.c)
C_SRC += $(wildcard util/*.c)

C_SRC += liblux/lux.c liblux/crc.c
C_SRC += $(wildcard BTrack/src/*.c)

OBJDIR = build
$(shell mkdir -p $(OBJDIR) >/dev/null)
OBJECTS = $(C_SRC:%.c=$(OBJDIR)/%.o)

# Compiler flags
INC = -I.

LIBRARIES = -lSDL2_ttf -lm -lportaudio -lportmidi -lsamplerate $(shell pkg-config --libs gl glu sdl2 fftw3)

CFLAGS = -std=gnu11 -ggdb3 -g -Ofast -O3 -march=native $(INC)

CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS += -D_POSIX_C_SOURCE=20160524
LFLAGS = $(CFLAGS)

# File dependency generation
DEPDIR = .deps
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPS = $(OBJECTS:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
-include $(DEPS)
$(DEPDIR)/%.d : %.c .deps
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -MM -MT $(@:$(DEPDIR)/%.d=%.o) >$@

# Targets
$(PROJECT): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(PROJECT) $(OBJECTS) $(LIBRARIES)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(APP_INC) -c -o $@ $<

.PHONY: all
all: $(PROJECT)

.PHONY: clean
clean:
	-rm -f $(PROJECT) tags
	-rm -rf $(OBJDIR) $(DEPDIR)

tags: $(C_SRC)
	ctags --exclude='beat-off/*' -R .

.DEFAULT_GOAL := all
