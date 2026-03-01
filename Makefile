# ────────────────────────────────────────────────────────────────
#  Compiler / Tools
# ────────────────────────────────────────────────────────────────

CXX 			= clang++
CXXFLAGS 		= -Wall -Wextra -g

BISONFLAGS 		= -d
LEXFLAGS		=


# ────────────────────────────────────────────────────────────────
#  Paths & Names
# ────────────────────────────────────────────────────────────────

EXEC			= vsopc

SRC				= main.cpp \
				  driver.cpp \
				  parser.cpp \
				  lexer.cpp

OBJ	  			= $(SRC:.cpp=.o)

BUILD_DIR		= build
SRC_DIR			= src

ARCHIVE			= vsopcompiler.tar.xz
ARCHIVE_FILES	= \
	driver.cpp driver.hpp \
	main.cpp \
	lexer.lex \
	parser.y \
	Makefile


# ────────────────────────────────────────────────────────────────
#  Main targets
# ────────────────────────────────────────────────────────────────


all: $(EXEC)


main.o: driver.hpp parser.hpp

driver.o: driver.hpp parser.hpp

parser.o: driver.hpp parser.hpp

lexer.o: driver.hpp parser.hpp

$(EXEC): $(OBJ)
	$(CXX) -o $@ $(LDFLAGS) $(OBJ)

parser.cpp: parser.y
	bison $(BISONFLAGS) -o parser.cpp $^

parser.hpp: parser.y
	bison $(BISONFLAGS) -o parser.cpp $^

lexer.cpp: lexer.lex
	flex $(LEXFLAGS) -o lexer.cpp $^

# ────────────────────────────────────────────────────────────────
#  Utility targets
# ────────────────────────────────────────────────────────────────

#archive: vsopcompiler.tar.gz

archive: $(ARCHIVE_FILES)
	tar -cJf $(ARCHIVE) --transform='s,^,vsopcompiler/,' $^

install-tools: 
	@:

clean:
	@rm -f $(EXEC)
	@rm -f $(OBJ)
	@rm -f lexer.cpp
	@rm -f parser.cpp parser.hpp location.hh
	@rm -f vsopcompiler.tar.xz

.PHONY: clean install-tools archive
