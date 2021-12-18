# Directory layout.
PROJDIR := $(realpath $(CURDIR)/)
SOURCEDIR := $(PROJDIR)/src
TESTDIR := $(PROJDIR)/tests
DOCDIR := $(PROJDIR)/doc

OBJDIR := $(PROJDIR)/obj
RELEASEDIR := $(PROJDIR)/release
DEBUGDIR := $(PROJDIR)/debug

# Target executable
TEST  = test
TEST_TARGET = $(BUILDDIR)/$(TEST)

CFLAGS = -Wall -Werror -std=c11 -I$(SOURCEDIR)
LDLIBS = -lpthread
ifeq ($(DEBUG),1)
	CFLAGS += -g
	LDLIBS +=
	BUILDDIR := $(DEBUGDIR)
else
	CFLAGS += -fPIC -O3 -DNDEBUG
	LDLIBS += -fPIC
	BUILDDIR := $(RELEASEDIR)
endif

# -------------------------------------------------------------------------------------------------

# glib - for testing
CFLAGS += `pkg-config --cflags glib-2.0`
LDLIBS += `pkg-config --libs glib-2.0`
# -------------------------------------------------------------------------------------------------

# Compiler and compiler options
CC = clang

# Show commands make uses
VERBOSE = TRUE

# Add this list to the VPATH, the place make will look for the source files
VPATH = $(SOURCEDIR)

# Create a list of *.c files in DIRS
SOURCES = $(wildcard $(SOURCEDIR)/*.c)
HEADERS = $(wildcard $(SOURCEDIR)/*.h)

# Define object files for all sources, and dependencies for all objects
OBJS := $(subst $(SOURCEDIR), $(BUILDDIR), $(SOURCES:.c=.o))
DEPS = $(OBJS:.o=.d)

# Hide or not the calls depending on VERBOSE
ifeq ($(VERBOSE),TRUE)
	HIDE = 
else
	HIDE = @
endif

.PHONY: all clean directories test doc

all: makefile directories $(TEST_TARGET)

$(TEST_TARGET): directories makefile $(OBJS) $(BUILDDIR)/$(TEST).o
	@echo Linking $@
	$(HIDE)$(CC) $(BUILDDIR)/$(TEST).o $(OBJS) $(LDLIBS) -o $@

$(BUILDDIR)/$(TEST).o: $(TESTDIR)/$(TEST).c
	@echo Building $@
	$(HIDE)$(CC) -c $(CFLAGS) -o $@ $< -MMD

-include $(DEPS)
-include $(BUILDDIR)/$(TEST).d

# Generate rules
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c makefile
	@echo Building $@
	$(HIDE)$(CC) -c $(CFLAGS) -o $@ $< -MMD

directories:
	@echo Creating directory $<
	$(HIDE)mkdir -p $(BUILDDIR) 2>/dev/null

test: directories makefile $(OBJS) $(BUILDDIR)/$(TEST).o
	@echo Linking $@
	$(HIDE)$(CC) $(BUILDDIR)/$(TEST).o $(OBJS) $(LDLIBS) -o $(TEST_TARGET)
	$(HIDE) $(TEST_TARGET)

doc: Doxyfile makefile $(SOURCES) $(HEADERS)
	$(HIDE) doxygen 2>/dev/null

clean:
	-$(HIDE)rm -rf $(DEBUGDIR) $(RELEASEDIR) $(DOCDIR) 2>/dev/null
	@echo Cleaning done!

#detected_OS = $(shell uname)
#ifeq ($(detected_OS), Linux)
#	target_dir = ~/usr/bin/
#else
#	target_dir = ~/bin/
#endif
#
#install: $(TARGET1) makefile
#	strip $(TARGET1) && cp $(TARGET1) $(target_dir)

