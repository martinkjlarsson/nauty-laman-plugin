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
Exec. time (1 core)   | 40 ms |  440 ms |    11 s   |   7.7 min  |      6.6 h      |
Exec. time (12 cores) | 20 ms |  140 ms |   2.1 s   |   1.3 min  |      1.1 h      |

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
