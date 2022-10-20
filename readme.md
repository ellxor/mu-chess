### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (290 Mnps)
kiwipete        193690690       (391 Mnps)
position 3      178633661       (238 Mnps)
position 4      706045033       (406 Mnps)
rotated 4       706045033       (406 Mnps)
position 5       89941194       (363 Mnps)
position 6      164075551       (420 Mnps)

Average: 359 Mnps
```

**TODO:**
- [x] Compress `bitbase.c` using raw string literals; note this already works with clang (C++) but not gcc.
- [ ] Add fancy magic bitboards to support processors without a fast BMI2 implementation
