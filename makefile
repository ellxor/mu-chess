CC     ?= gcc
CFLAGS ?= -O3 -march=native -s -Wall -Wextra -Wno-missing-field-initializers -Wno-pointer-sign
SRC     = src/main.c src/bitbase.c

default:
	$(CC) $(CFLAGS) $(SRC)

pgo:
	$(CC) $(CFLAGS) $(SRC) -fprofile-generate
	./a.out
	$(CC) $(CFLAGS) $(SRC) -fprofile-use

clang-pgo:
	sed 's/"\\r"/ "\\r" /g' src/bitbase.c > src/bitbase.cc
	clang src/main.c src/bitbase.cc -O3 -march=native -w -fprofile-generate
	./a.out
	llvm-profdata merge *.profraw -o default.profdata
	clang src/main.c src/bitbase.cc -O3 -march=native -w -fprofile-use
	rm -rf *.prof* src/bitbase.cc
