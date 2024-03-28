/*
    Usage: filter_sparse K L N [-u]

    Filter graphs to keep the ones that are (K,L)-sparse. A graph is (K,L)-
    sparse if every subgraph with n > N vertices has at most Kn-L edges, and
    (K,L)-tight if it is (K,L)-sparse and has exactly Kn-L edges.
    -u suppresses the ouput and only counts the sparse graphs.
*/

// gcc -O4 -mpopcnt -march=native filter_sparse.c ../nauty27r3/gtools.c -o filter_sparse

#define MAXN 32

#include "../nauty27r3/gtools.h"

#define NTH_NODE(n) (bit[n])
#define CTZ(x) __builtin_ctz(x)

boolean is_sparse(graph *g, int n, int K, int L, int N)
{
    int i, j, sn, sm, degree;
    setword mask;

    mask = 0;
    sn = 0;
    sm = 0;

    for (i = 1; i < (1 << n); ++i)
    {
        j = CTZ(i);
        mask ^= NTH_NODE(j); /* add or remove node */
        degree = POPCOUNT(g[j] & mask);
        sn += mask & NTH_NODE(j) ? 1 : -1;
        sm += mask & NTH_NODE(j) ? degree : -degree;

        if (sn > N && sm > K * sn - L)
            return FALSE;
    }
    return TRUE;
}

int main(int argc, const char *argv[])
{
    int n, K, L, N, count_total, count_sparse;
    graph g[MAXN];
    char *line;
    size_t size;
    boolean nooutput;

    if (argc < 4)
        gt_abort(">E filter_sparse: K, L, and N are mandatory arguments\n");

    K = atoi(argv[1]);
    L = atoi(argv[2]);
    N = atoi(argv[3]);

    if (K < 1)
        gt_abort(">E filter_sparse: K has to be a positive integer\n");
    if (N < 1)
        gt_abort(">E filter_sparse: N has to be a positive integer\n");

    nooutput = argc >= 5 && strcmp(argv[4], "-u") == 0;

    count_total = 0;
    count_sparse = 0;

    while (getline(&line, &size, stdin) != -1)
    {
        n = graphsize(line);
        if (n > MAXN)
            gt_abort(">E filter_sparse: graph has too many vertices\n");

        stringtograph(line, g, 1); // Row of adjacency matrix fits in one word.

        if (is_sparse(g, n, K, L, N))
        {
            if (!nooutput)
                printf("%s", line);
            count_sparse++;
        }

        count_total++;
    }

    fprintf(stderr, ">Z %d/%d graphs were sparse (K,L,N) = (%d,%d,%d)\n", count_sparse, count_total, K, L, N);

    return 0;
}
