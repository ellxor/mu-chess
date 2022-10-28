### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (418 Mnps)
kiwipete        193690690       (550 Mnps)
position 3      178633661       (309 Mnps)
position 4      706045033       (522 Mnps)
rotated 4       706045033       (522 Mnps)
position 5       89941194       (516 Mnps)
position 6      164075551       (524 Mnps)

Average: 480 Mnps
```
