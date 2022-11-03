### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       ( 875 Mnps)
kiwipete        193690690       (1435 Mnps)
position 3      178633661       ( 650 Mnps)
position 4      706045033       (1300 Mnps)
rotated 4       706045033       (1274 Mnps)
position 5       89941194       (1323 Mnps)
position 6      164075551       (1292 Mnps)
promotions       71179139       ( 837 Mnps)

Average: 1123 Mnps
```
