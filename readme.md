### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       ( 799 Mnps)
kiwipete        193690690       (1601 Mnps)
position 3      178633661       ( 679 Mnps)
position 4      706045033       (1293 Mnps)
rotated 4       706045033       (1300 Mnps)
position 5       89941194       (1363 Mnps)
position 6      164075551       (1402 Mnps)

Average: 1205 Mnps
```
