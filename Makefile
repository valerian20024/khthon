# -----------------------------------------------------------------------------
#  Compiler / Tools
# -----------------------------------------------------------------------------

CXX        = clang++
CXXFLAGS   = -Wall -Wextra -g -std=c++17

BISONFLAGS = -d
FLEXFLAGS  =

# -----------------------------------------------------------------------------
#  Paths
# -----------------------------------------------------------------------------

EXEC       = vsopc
SRC_DIR    = src
BUILD_DIR  = build

ARCHIVE    = vsopcompiler.tar.xz

# -----------------------------------------------------------------------------
#  Sources
# -----------------------------------------------------------------------------

SRC        = main.cpp driver.cpp ast.cpp parser.cpp lexer.cpp semantics.cpp
OBJ        = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))

# -----------------------------------------------------------------------------
#  Archive
#-----------------------------------------------------------------------------

ARCHIVE_FILES = \
	$(SRC_DIR)/driver.cpp \
	$(SRC_DIR)/driver.hpp \
	$(SRC_DIR)/ast.hpp \
	$(SRC_DIR)/ast.cpp \
	$(SRC_DIR)/semantics.hpp\
	$(SRC_DIR)/semantics.cpp\
	$(SRC_DIR)/colors.hpp \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/lexer.lex \
	$(SRC_DIR)/parser.y \
	Makefile

# -----------------------------------------------------------------------------
#  Colors
#-----------------------------------------------------------------------------

# ANSI escape sequences. 
# 	38: foreground
#	5: extended 256 colors space
#	e.g. 203: the color code

C_BLUE    	:= "\e[38;5;27m"
C_GREEN     := "\e[38;5;35m"
C_RED    	:= "\e[38;5;85m"
C_LAVANDER  := "\e[38;5;141m"
C_FUCHSIA   := "\e[38;5;161m"
C_SALMON    := "\e[38;5;203m"
C_GOLDEN	:= "\e[38;5;208m"
C_PEACH     := "\e[38;5;216m"
C_END 		:= "\e[0m"

# -----------------------------------------------------------------------------
#  Main targets
# -----------------------------------------------------------------------------

all: $(EXEC)

# Link
$(EXEC): $(OBJ)
	@echo -e $(C_LAVANDER)"Linking..."$(C_END);
	$(CXX) -o $@ $(LDFLAGS) $(OBJ)

# Compile (generic rule for all .cpp -> build/*.o)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo -e $(C_LAVANDER)"Compiling..."$(C_END);
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build dir if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# -----------------------------------------------------------------------------
#  Code generation (Bison / Flex)
# -----------------------------------------------------------------------------

$(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.hpp: $(SRC_DIR)/parser.y
	@echo -e $(C_PEACH)"Bison..."$(C_END);
	bison $(BISONFLAGS) -o $(SRC_DIR)/parser.cpp $^

$(SRC_DIR)/lexer.cpp: $(SRC_DIR)/lexer.lex
	@echo -e $(C_PEACH)"Flex..."$(C_END);
	flex $(FLEXFLAGS) -o $(SRC_DIR)/lexer.cpp $^

# -----------------------------------------------------------------------------
#  Per-file header dependencies
# -----------------------------------------------------------------------------

$(BUILD_DIR)/main.o:		$(SRC_DIR)/main.cpp \
							$(SRC_DIR)/driver.hpp \
							$(SRC_DIR)/parser.hpp
$(BUILD_DIR)/driver.o:		$(SRC_DIR)/driver.cpp \
							$(SRC_DIR)/driver.hpp \
							$(SRC_DIR)/parser.hpp \
							$(SRC_DIR)/ast.hpp \
							$(SRC_DIR)/colors.hpp \
							$(SRC_DIR)/semantics.hpp
$(BUILD_DIR)/parser.o:		$(SRC_DIR)/parser.cpp \
							$(SRC_DIR)/driver.hpp \
							$(SRC_DIR)/parser.hpp
$(BUILD_DIR)/lexer.o: 		$(SRC_DIR)/lexer.cpp \
							$(SRC_DIR)/driver.hpp \
							$(SRC_DIR)/parser.hpp
$(BUILD_DIR)/ast.o:			$(SRC_DIR)/ast.cpp \
							$(SRC_DIR)/ast.hpp \
							$(SRC_DIR)/location.hh
$(BUILD_DIR)/semantics.o:	$(SRC_DIR)/semantics.cpp \
							$(SRC_DIR)/semantics.hpp \
							$(SRC_DIR)/ast.hpp \
							$(SRC_DIR)/driver.hpp

# -----------------------------------------------------------------------------
#  Utility targets
# -----------------------------------------------------------------------------

# Add a root directory called vscompiler in the archive
archive: $(ARCHIVE_FILES)
	@echo -e $(C_GREEN)"Creating archive..."$(C_END);
	@tar -cJf $(ARCHIVE) --transform='s,^,vsopcompiler/,' $^

install-tools:
	@:

clean:
	@rm -f $(EXEC)
	@rm -f $(BUILD_DIR)/*.o
	@rm -f $(SRC_DIR)/lexer.cpp $(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.hpp $(SRC_DIR)/location.hh
	@rm -f $(ARCHIVE)

debug: BISONFLAGS += -Wcounterexamples

debug: $(EXEC)
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"

.PHONY: all clean install-tools archive debug
