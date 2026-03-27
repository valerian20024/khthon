# -----------------------------------------------------------------------------
#  Compiler / Tools
# -----------------------------------------------------------------------------

CXX        = clang++
CXXFLAGS   = -Wall -Wextra -g -std=c++17

BISONFLAGS = -d
FLEXFLAGS  =

# -----------------------------------------------------------------------------
#  Paths & Names
# -----------------------------------------------------------------------------

EXEC       = vsopc
SRC_DIR    = src
BUILD_DIR  = build

ARCHIVE    = vsopcompiler.tar.xz

# -----------------------------------------------------------------------------
#  Sources & Objects
# -----------------------------------------------------------------------------

SRC        = main.cpp driver.cpp ast.cpp parser.cpp lexer.cpp
OBJ        = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))

#
#  Archive
#

ARCHIVE_FILES = \
	$(SRC_DIR)/driver.cpp \
	$(SRC_DIR)/driver.hpp \
	$(SRC_DIR)/ast.hpp \
	$(SRC_DIR)/ast.cpp \
	$(SRC_DIR)/colors.hpp \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/lexer.lex \
	$(SRC_DIR)/parser.y \
	Makefile

# -----------------------------------------------------------------------------
#  Main targets
# -----------------------------------------------------------------------------

all: $(EXEC)

# Link
$(EXEC): $(OBJ)
	$(CXX) -o $@ $(LDFLAGS) $(OBJ)

# Compile (generic rule for all .cpp -> build/*.o)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build dir if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# -----------------------------------------------------------------------------
#  Code generation (Bison / Flex)
# -----------------------------------------------------------------------------

$(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.hpp: $(SRC_DIR)/parser.y
	bison $(BISONFLAGS) -o $(SRC_DIR)/parser.cpp $^

$(SRC_DIR)/lexer.cpp: $(SRC_DIR)/lexer.lex
	flex $(FLEXFLAGS) -o $(SRC_DIR)/lexer.cpp $^

# -----------------------------------------------------------------------------
#  Per-file header dependencies
# -----------------------------------------------------------------------------

$(BUILD_DIR)/main.o:   $(SRC_DIR)/main.cpp   $(SRC_DIR)/driver.hpp $(SRC_DIR)/parser.hpp
$(BUILD_DIR)/driver.o: $(SRC_DIR)/driver.cpp $(SRC_DIR)/driver.hpp $(SRC_DIR)/parser.hpp $(SRC_DIR)/colors.hpp
$(BUILD_DIR)/parser.o: $(SRC_DIR)/parser.cpp $(SRC_DIR)/driver.hpp $(SRC_DIR)/parser.hpp
$(BUILD_DIR)/lexer.o:  $(SRC_DIR)/lexer.cpp  $(SRC_DIR)/driver.hpp $(SRC_DIR)/parser.hpp
$(BUILD_DIR)/ast.o:    $(SRC_DIR)/ast.cpp    $(SRC_DIR)/ast.hpp    $(SRC_DIR)/location.hh

# -----------------------------------------------------------------------------
#  Utility targets
# -----------------------------------------------------------------------------

#archive:
#	tar -cJf $(ARCHIVE) \
#	    --transform='s,^$(SRC_DIR)/,vsopcompiler/,' \
#	    $(addprefix $(SRC_DIR)/, \
#	        driver.cpp driver.hpp \
#	        ast.hpp ast.cpp \
#	        colors.hpp \
#	        main.cpp \
#	        lexer.lex \
#	        parser.y) \
#	    --transform='s,^,vsopcompiler/,' \
#	    Makefile

# Add a root directory called vscompiler in the archive
archive: $(ARCHIVE_FILES)
	tar -cJf $(ARCHIVE) --transform='s,^,vsopcompiler/,' $^


install-tools:
	@:

clean:
	@rm -f $(EXEC)
	@rm -f $(BUILD_DIR)/*.o
	@rm -f $(SRC_DIR)/lexer.cpp
	@rm -f $(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.hpp $(SRC_DIR)/location.hh
	@rm -f $(ARCHIVE)

debug:
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"

.PHONY: all clean install-tools archive debug