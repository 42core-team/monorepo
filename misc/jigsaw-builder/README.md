# Jigsaw World Generator

> Mix of random and manually put together world assets, which makes cool maps.

## Template Format:

- X -> Wall
- R -> Resource
- M -> Money
- 0123456789 -> Wall with certain likelihood. (1 -> 10%, 9 -> 90%)
- abcdefghij -> Resource with certain likelihood. (b -> 10%, j -> 90%)
- ABCDEFGHIJ -> Money with certain likelihood. (B -> 10%, J -> 90%)
- klmnopqrst -> Wall, if not resource (l -> 10%, t -> 90% )
- KLNOPQSTUV -> Resource, if not money (L -> 10%, V -> 90%)
- uvwxyz!/$% -> Money, if not wall (v -> 10%, ß -> 90 % )

Please note that template may be randomly mirrored and rotated before being displayed in-game.

> if you open the template_edit.html file in your browser, theres a simple editor that makes all this pretty intuitive i think

| chance | wall | resource | wall otherwise resource | money otherwise wall | resource otherwise money |
| ------ | ---- | -------- | ----------------------- | -------------------- | ------------------------ |
| 100%   | X    | R        | -                       | -                    | -                        |
| 90%    | 9    | j        | t                       | %                    | V                        |
| 80%    | 8    | i        | s                       | $                    | U                        |
| 70%    | 7    | h        | r                       | /                    | T                        |
| 60%    | 6    | g        | q                       | !                    | S                        |
| 50%    | 5    | f        | p                       | z                    | Q                        |
| 40%    | 4    | e        | o                       | y                    | P                        |
| 30%    | 3    | d        | n                       | x                    | O                        |
| 20%    | 2    | c        | m                       | w                    | N                        |
| 10%    | 1    | b        | l                       | v                    | L                        |
| 0 %    | 0    | a        | k                       | u                    | K                        |

> 0% chances are currently unused by may be helpful if we want to add resource respawning etc
