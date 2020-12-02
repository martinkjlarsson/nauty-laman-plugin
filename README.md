# Nauty Laman plugin
A plugin for the `geng` program provided with [Nauty](http://pallini.di.uniroma1.it/) that enables quick generation of [Laman graphs](https://en.wikipedia.org/wiki/Laman_graph).

## Installation
1. Download and build [Nauty](http://pallini.di.uniroma1.it/).
2. Clone this repo and copy `prunelaman.h` and `count_laman` to the folder containing Nauty.
3. `geng` has to be rebuilt with the flag `-D'PLUGIN="prunelaman.h"'` to inject the plugin. Add the flag, for example, by manually editing the `makefile` and then rebuild `geng` (e.g. `rm -f geng && make geng`).

## Execution time
The table below shows the execution time when counting the number of Laman graphs for various number of vertices n. The time was measured running on a single core of an Intel® Core™ i7-6800K CPU @ 3.40GHz.
 n | Time/s
---|-------
<8 | 0.000
 8 | 0.004
 9 | 0.060
10 | 0.828
11 | 27.5
12 | 1280

## Examples

### Count all Laman graphs on 10 vertices
```shell
./geng 10 -u
```
```shell
>A ./geng -d2D9 n=10 e=17
>Z 110132 graphs generated in 0.95 sec
```

### Split the work over four instances
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
