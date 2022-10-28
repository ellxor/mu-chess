CC       = clang
CFLAGS   = -O3 -march=native -s -o perft
WARNINGS = -Wall -Wextra -Wno-invalid-source-encoding
SRC      = src/main.c -xc++ src/bitbase.c

default:
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC)

debug:
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC) -fsanitize=undefined

clang-pgo:
	clang $(CFLAGS) $(WARNINGS) $(SRC) -fprofile-generate
	./perft
	llvm-profdata merge *.profraw -o default.profdata
	clang $(CFLAGS) $(WARNINGS) $(SRC) -fprofile-use
	rm -rf *.profraw default.profdata
