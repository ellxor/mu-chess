CC       = clang
CFLAGS   = -O3 -march=native -s -o perft
WARNINGS = -Wall -Wextra
SRC      = src/main.c

default:
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC)

debug:
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC) -fsanitize=undefined

clang-pgo:
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC) -fprofile-generate
	./perft
	llvm-profdata merge *.profraw -o default.profdata
	$(CC) $(CFLAGS) $(WARNINGS) $(SRC) -fprofile-use
	rm -rf *.profraw default.profdata
