CC     ?= gcc
CFLAGS ?= -O3 -march=native -s -Wall -Wextra -Wno-missing-field-initializers -Wno-pointer-sign
SRC     = src/main.c src/bitbase.c

default:
	$(CC) $(CFLAGS) $(SRC)

pgo:
	$(CC) $(CFLAGS) $(SRC) -fprofile-generate
	./a.out
	$(CC) $(CFLAGS) $(SRC) -fprofile-use
