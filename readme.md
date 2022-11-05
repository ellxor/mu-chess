### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (504 Mnps)
kiwipete        193690690       (596 Mnps)
position 3      178633661       (372 Mnps)
position 4      706045033       (567 Mnps)
rotated 4       706045033       (563 Mnps)
position 5       89941194       (604 Mnps)
position 6      164075551       (588 Mnps)

Average: 542 Mnps
```
