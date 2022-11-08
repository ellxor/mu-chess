### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (476 Mnps)
kiwipete        193690690       (613 Mnps)
position 3      178633661       (370 Mnps)
position 4      706045033       (561 Mnps)
rotated 4       706045033       (557 Mnps)
position 5       89941194       (612 Mnps)
position 6      164075551       (599 Mnps)

Average: 541 Mnps
```
