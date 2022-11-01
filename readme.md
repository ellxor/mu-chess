### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       ( 850 Mnps)
kiwipete        193690690       (1424 Mnps)
position 3      178633661       ( 695 Mnps)
position 4      706045033       (1277 Mnps)
rotated 4       706045033       (1307 Mnps)
position 5       89941194       (1323 Mnps)
position 6      164075551       (1452 Mnps)

Average: 1190 Mnps
```
