
/* Add this flag when compiling: -D'PLUGIN="prunelaman.h"' */

#define EMBED_DIM 2
#define EMBED_DOF (EMBED_DIM * (EMBED_DIM + 1) / 2)
#define HIGHEST_BIT(type) (~((type)-1 >> 1))

#define PRUNE prunelaman
#define PREPRUNE preprunelaman

/* Note: PLUGIN_INIT happens after validation of the input arguments in geng.c.
 * Beware of illegal argument combinations. */
#define PLUGIN_INIT                                                         \
    if (!gote)                                                              \
    {                                                                       \
        geng_mine = geng_maxe = mine = maxe = EMBED_DIM * maxn - EMBED_DOF; \
    }                                                                       \
    if (!gotd)                                                              \
    {                                                                       \
        geng_mindeg = mindeg = EMBED_DIM;                                   \
    }

int prunelaman(graph *g, int n, int maxn)
{
    int i, j, k, l;
    int subgraphsleft;
    int nodeinds[MAXN] = {0};
    graph mask;

    /* subgraphs are dealt with in pre-pruning */
    if (n < maxn)
        return FALSE;

    /* go through all subgraphs of size k to make sure m <= EMBED_DIM*n-EMBED_DOF */
    for (k = n - 1; k > EMBED_DIM + 1; --k)
    {
        /* init subgraph with the k first nodes */
        for (i = 0; i < k; ++i)
            nodeinds[i] = i;

        subgraphsleft = TRUE;
        while (subgraphsleft)
        {
            /* mask out all nodes not in the subgraph */
            mask = 0;
            for (i = 0; i < k; ++i)
                mask |= HIGHEST_BIT(graph) >> nodeinds[i];

            /* find number of edges in subgraph */
            l = 0;
            for (i = 0; i < k; ++i)
                l += POPCOUNT(g[nodeinds[i]] & mask);
            l = l / 2;

            /* subgraph is over-determined => not minimal */
            if (l > EMBED_DIM * k - EMBED_DOF)
                return TRUE;

            /* go to next subgraph */
            for (i = k - 1; i >= 0; --i)
            {
                nodeinds[i]++;
                if (nodeinds[i] >= n + i - k + 1)
                {
                    /* wrap, continue and increase next index */
                    if (i == 0)
                        subgraphsleft = FALSE;
                }
                else
                {
                    for (j = i + 1; j < k; ++j)
                        nodeinds[j] = nodeinds[i] - i + j;
                    break;
                }
            }
        }
    }
    return FALSE;
}

int preprunelaman(graph *g, int n, int maxn)
{
    int i, j, m;

    /* graphs with n <= EMBED_DIM+1 cannot be over-determined */
    if (n <= EMBED_DIM + 1 && n < maxn)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* graph is over-determined => not minimal */
    if (n > EMBED_DIM + 1 && m > EMBED_DIM * n - EMBED_DOF)
        return TRUE;

    if (n == maxn)
    {
        /* fully connected graph is rigid */
        if (m == n * (n - 1) / 2)
            return FALSE;

        /* graph is under-determined => not rigid */
        if (m != EMBED_DIM * n - EMBED_DOF)
            return TRUE;
    }
    return FALSE;
}
