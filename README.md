# Nauty Laman plugin
A plugin for the `geng` program provided with [Nauty](http://pallini.di.uniroma1.it/) that enables quick generation of [Laman graphs](https://en.wikipedia.org/wiki/Laman_graph).


## Installation
1. Download and build [Nauty](http://pallini.di.uniroma1.it/).
2. Clone this repo and copy `prunelaman.h` and `count_laman` to the folder containing Nauty.
3. `geng` has to be rebuilt with the flag `-D'PLUGIN="prunelaman.h"'` to inject the plugin. Add the flag, for example, by manually editing the `makefile` and then rebuild `geng` (e.g. `rm -f geng && make geng`).


## Results - counts and execution times
The tables below show the execution time when generating graphs for various number of vertices n. Unless otherwise specified, the tests were run on an 6-core/12-thread Intel® Core™ i7-6800K CPU @ 3.40GHz. Extensions to entries in [OEIS](https://oeis.org/) are marked with *(new)*.


### Laman graphs
See OEIS entry [A227117](https://oeis.org/A227117). When increasing n by one, for large n, the number of graphs increases by a factor of approximately 30 while the execution time increases by a factor of approximately 60. `geng` parallelizes well over physical cores but poorly over logical cores.
n                     |    10   |     11    |     12     |        13       |    14 (new)    |      15 (new)     |
----------------------|:-------:|:---------:|:----------:|:---------------:|:--------------:|:-----------------:|
Laman graphs          | 110 132 | 2 039 273 | 44 176 717 |  1 092 493 042  | 30 322 994 747 | 932 701 249 291\* |
Exec. time (1 core)   |  440 ms |    11 s   |   7.7 min  |      6.6 h      | *Not measured* |   *Not measured*  |
Exec. time (6 cores)  |  180 ms |   2.5 s   |   1.4 min  |      1.2 h      | *Not measured* |   *Not measured*  |
Exec. time (12 cores) |  140 ms |   2.1 s   |   1.3 min  |      1.1 h      |    2.5 days    |   *Not measured*  |

\* Running 128 instances of `geng` on an AMD Ryzen Threadripper 3990X 64-Core Processor for 14 days.


### Geiringer graphs
See OEIS entry [A328419](https://oeis.org/A328419). The embedding dimension can be changed by setting the macro `EMBED_DIM` in `prunelaman.h` (default value is 2). By setting it to 3, we can generate minimally rigid graphs in 3D, i.e., Geiringer graphs. However, the generation will only be accurate for n=1..7. For larger n we also generate some flexible graphs, e.g., the double-banana graph for n=8. The generated graphs are nonetheless useful as the flexible ones can be filtered out later using other methods.
n                     |   6   |   7   |   8   |    9   |    10   |     11     |        12       |
----------------------|:-----:|:-----:|:-----:|:------:|:-------:|:----------:|:---------------:|
Generated graphs      |   4   |   26  |  375  | 11 495 | 613 092 | 48 185 341 |  5 116 473 573  |
Geiringer graphs      |   4   |   26  |  374  | 11 487 | 612 884 | 48 176 183 | 5 115 840 190\* |
Exec. time (1 core)   |       |       |       |  50 ms |  1.7 s  |   3.8 min  |       13 h      |

\* Calculated by numerically checking the rigidity of the generated graphs.


### Laman graphs constructed by Henneberg type I moves
See OEIS entry [A273468](https://oeis.org/A273468). By setting the macro `PRUNE` (default value `prunelaman`) to `prunehenneberg1` we can generate all Laman graphs that can be constructed using only [Henneberg type I moves](https://en.wikipedia.org/wiki/Laman_graph#Henneberg_construction).
n                     |   10   |     11    |     12     |      13     |    14 (new)    |     15 (new)    |
----------------------|:------:|:---------:|:----------:|:-----------:|:--------------:|:---------------:|
Laman graphs          | 75 635 | 1 237 670 | 23 352 425 | 498 028 767 | 11 836 515 526 | 310 152 665 647 |
Exec. time (1 core)   | 140 ms |   1.3 s   |    24 s    |    11 min   |      5.4 h     |    7.7 days\*   |

\* Total CPU time of 6 cores.


### Bipartite Laman graphs
See OEIS entry [A328060](https://oeis.org/A328060). By setting the option `-b` for `geng` we can generate bipartite Laman graphs.
n                      |   12   |   13   |     14    |     15     |   16 (new)  |    17 (new)   |
-----------------------|:------:|:------:|:---------:|:----------:|:-----------:|:-------------:|
Bipartite Laman graphs |  8 304 | 92 539 | 1 210 044 | 17 860 267 | 293 210 063 | 5 277 557 739 |
Exec. time (1 core)    | 320 ms |  3.4 s |    66 s   |   29 min   |     14 h    |   21 days\*   |

\* Total CPU time of 6 cores.


## Examples

### Count all Laman graphs on 11 vertices
```
./geng 11 -u
```
```
>A ./geng -d2D10 n=11 e=19
>Z 2039273 graphs generated in 14.71 sec
```

### Split the work over four instances running in parallel
```
./count_laman 11 4
```
```
>A ./geng -X1x4000d2D10 n=11 e=19 class=0/4
>A ./geng -X1x4000d2D10 n=11 e=19 class=1/4
>A ./geng -X1x4000d2D10 n=11 e=19 class=2/4
>A ./geng -X1x4000d2D10 n=11 e=19 class=3/4
>Z 497693 graphs generated in 8.00 sec
>Z 490084 graphs generated in 7.85 sec
>Z 502422 graphs generated in 8.10 sec
>Z 549074 graphs generated in 8.36 sec
```
Summing up the results from the four instances yields the same result as above.
