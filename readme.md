### MuChess

This is a simple chess implementation using compressed bitboards to store a chess position in 256 bits. It is an improvement upon [uchess](https://github.com/ellxor/uchess) and also removes the `pext` dependency.

**Results:**
```
startpos        119060324       (467 Mnps)
kiwipete        193690690       (568 Mnps)
position 3      178633661       (354 Mnps)
position 4      706045033       (549 Mnps)
rotated 4       706045033       (549 Mnps)
position 5       89941194       (573 Mnps)
position 6      164075551       (584 Mnps)

Average: 520 Mnps
```
