
/* Add this flag when compiling: -D'PLUGIN="prunelaman.h"' */

#define EMBED_DIM 2
#define EMBED_DOF (EMBED_DIM * (EMBED_DIM + 1) / 2)
#define HIGHEST_BIT(type) (~((type)-1 >> 1))

#define PRUNE prunelaman

/* Note: PLUGIN_INIT happens after validation of the input arguments in geng.c.
 * Beware of illegal argument combinations. */
#define PLUGIN_INIT                                                         \
    if (!gote && maxn > 1)                                                  \
    {                                                                       \
        geng_mine = geng_maxe = mine = maxe = EMBED_DIM * maxn - EMBED_DOF; \
    }                                                                       \
    if (!gotd && maxn > EMBED_DIM)                                          \
    {                                                                       \
        geng_mindeg = mindeg = EMBED_DIM;                                   \
    }

int prunelaman(graph *g, int n, int maxn)
{
    int i, j, k, l, m;
    int subgraphsleft;
    int nodeinds[MAXN];
    graph mask;

    /* subgraphs with n <= EMBED_DIM+1 cannot be overdetermined
     * graph underdeterminedness is prevented using mine and maxe */
    if (n <= EMBED_DIM + 1)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not minimal */
    if (m > EMBED_DIM * n - EMBED_DOF)
        return TRUE;

    /* graph is underdetermined => not rigid */
    if (n == maxn && m != EMBED_DIM * n - EMBED_DOF)
        return TRUE;

    /* Go through all subgraphs to make sure m <= EMBED_DIM*n-EMBED_DOF.
     * geng constructs graphs by succesively adding more nodes. Therefore,
     * we only need to check the subgraphs containing the new node. The other
     * subgraphs have been checked in previous steps. */
    for (k = n - 1; k > EMBED_DIM + 1; --k)
    {
        /* init subgraph with the k-1 first nodes and the new node */
        for (i = 0; i < k - 1; ++i)
            nodeinds[i] = i;
        nodeinds[k - 1] = n - 1;

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

            /* subgraph is overdetermined => not minimal */
            if (l > EMBED_DIM * k - EMBED_DOF)
                return TRUE;

            /* go to next subgraph */
            for (i = k - 2; i >= 0; --i)
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
                    for (j = i + 1; j < k - 1; ++j)
                        nodeinds[j] = nodeinds[i] - i + j;
                    break;
                }
            }
        }
    }
    return FALSE;
}
