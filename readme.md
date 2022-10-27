### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (411 Mnps)
kiwipete        193690690       (541 Mnps)
position 3      178633661       (307 Mnps)
position 4      706045033       (511 Mnps)
rotated 4       706045033       (511 Mnps)
position 5       89941194       (511 Mnps)
position 6      164075551       (513 Mnps)

Average: 473 Mnps
```

**TODO:**
- [x] Compress `bitbase.c` using raw string literals; note this already works with clang (C++) but not gcc.
- [ ] Add fancy magic bitboards to support processors without a fast BMI2 implementation
