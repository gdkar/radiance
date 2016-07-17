PROJECT = radiance
#CC = gcc
#CC = clang

# Source files
SRC_DIRS = . \
		   audio \
		   midi \
		   output \
		   pattern \
		   time \
		   ui \
		   util \
		   liblux \
		   BTrack/src

C_SRC  = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
CXX_SRC_  = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
CXX_SRC_ += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cxx))
CXX_SRC  = $(strip $(CXX_SRC_))
#C_SRC  = $(wildcard *.c)
#C_SRC += $(wildcard audio/*.c)
#C_SRC += $(wildcard midi/*.c)
#C_SRC += $(wildcard output/*.c)
#C_SRC += $(wildcard pattern/*.c)
#C_SRC += $(wildcard time/*.c)
#C_SRC += $(wildcard ui/*.c)
#C_SRC += $(wildcard util/*.c)
#
#C_SRC += $(wildcard liblux/*.c)
#C_SRC += $(wildcard BTrack/src/*.c)
#
#CXX_SRC = $(wildcard *.cpp)
#CXX_SRC = $(wildcard audio/*.cpp)
OBJDIR = build
$(shell mkdir -p $(OBJDIR) >/dev/null)
OBJECTS = $(C_SRC:%.c=$(OBJDIR)/%.o)

# Compiler flags
INC = -I.

LIBRARIES = -lSDL2 -lSDL2_ttf -lGL -lGLU -lm -lportaudio -lportmidi -lfftw3 -lsamplerate $(shell pkg-config --libs glfw3 gl glew)

CFLAGS = -std=gnu11 
CXXFLAGS = -std=gnu++17

CPPFLAGS+= $(shell pkg-config --cflags glfw3 gl glew) -shared
OPTFLAGS =  -shared -ggdb3 -O3 $(INC)
OPTFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
OPTFLAGS += -D_POSIX_C_SOURCE=20160524

LFLAGS = $(CFLAGS)
CFLAGS   += $(OPTFLAGS) -march=native -fPIC
CXXFLAGS += $(OPTFLAGS) -march=native

# File dependency generation
DEPDIR = .deps
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPS = $(OBJECTS:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
-include $(DEPS)

show-config:
	@echo "CC: $(CC)"
	@echo "CXX: $(CXX)"
	@echo "LD : $(LD)"
	@echo "C_SRC: $(C_SRC)"
	@echo "CXX_SRC: $(CXX_SRC)"
$(DEPDIR)/%.d : %.c .deps
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -MM -MT $(@:$(DEPDIR)/%.d=%.o) >$@

$(DEPDIR)/%.d : %.cpp .deps
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:$(DEPDIR)/%.d=%.o) >$@

$(DEPDIR)/%.d : %.cxx .deps
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:$(DEPDIR)/%.d=%.o) >$@

LD=$(if $(CXX_SRC),$(CXX),$(CC))


# Targets
$(PROJECT): $(OBJECTS)
	$(LD) $(LFLAGS) -o $(PROJECT) $(OBJECTS) $(LIBRARIES)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(APP_INC) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(APP_INC) -c -o $@ $<

$(OBJDIR)/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(APP_INC) -c -o $@ $<

.PHONY: all
all: $(PROJECT)

.PHONY: clean
clean:
	-rm -f $(PROJECT) tags
	-rm -rf $(OBJDIR) $(DEPDIR)

tags: $(C_SRC) $(CXX_SRC)
	ctags --exclude='beat-off/*' -R .

.DEFAULT_GOAL := all
