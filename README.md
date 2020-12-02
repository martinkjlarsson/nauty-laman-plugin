# Nauty Laman plugin
A plugin for the `geng` program provided with [Nauty](http://pallini.di.uniroma1.it/) that enables quick generation of [Laman graphs](https://en.wikipedia.org/wiki/Laman_graph).

## Installation
1. Download and build [Nauty](http://pallini.di.uniroma1.it/).
2. Clone this repo and copy `prunelaman.h` and `count_laman` to the folder containing Nauty.
3. `geng` has to be rebuilt with the flag `-D'PLUGIN="prunelaman.h"'` to inject the plugin. Add the flag, for example, by manually editing the `makefile` and then rebuild `geng` (e.g. `rm -f geng && make geng`).

## Execution time
The table below shows the execution time when counting the number of Laman graphs for various number of vertices n. The time was measured running on a single core as well as on 6 cores/12 threads of an Intel® Core™ i7-6800K CPU @ 3.40GHz. The number of Laman graphs for n=1..13 can be found in the OEIS entry [A227117](https://oeis.org/A227117).
n                     |   9   |    10   |     11    |     12     |        13       |
----------------------|:-----:|:-------:|:---------:|:----------:|:---------------:|
\# Laman graphs       | 7 222 | 110 132 | 2 039 273 | 44 176 717 |  1 092 493 042  |
Exec. time (1 core)   | 60 ms |  820 ms |    28 s   |    21 min  | *Not measured*  |
Exec. time (12 cores) | 30 ms |  220 ms |   4.4 s   |   3.1 min  |      2.6 h      |

## Examples

### Count all Laman graphs on 10 vertices
```shell
./geng 10 -u
```
```shell
>A ./geng -d2D9 n=10 e=17
>Z 110132 graphs generated in 0.95 sec
```

### Split the work over four instances running in parallel
```
./count_laman 10 4
```
```
>A ./geng -X0x200d2D9 n=10 e=17 class=0/4
>A ./geng -X0x200d2D9 n=10 e=17 class=1/4
>A ./geng -X0x200d2D9 n=10 e=17 class=3/4
>A ./geng -X0x200d2D9 n=10 e=17 class=2/4
>Z 17790 graphs generated in 0.28 sec
>Z 21790 graphs generated in 0.31 sec
>Z 28070 graphs generated in 0.39 sec
>Z 42482 graphs generated in 0.48 sec
```
Summing up the results from the four instances yields the same result as above.
