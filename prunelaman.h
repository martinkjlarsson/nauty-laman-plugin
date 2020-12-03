
/* Add this flag when compiling: -D'PLUGIN="prunelaman.h"' */

#define EMBED_DIM 2
#define EMBED_DOF (EMBED_DIM * (EMBED_DIM + 1) / 2)
#define FIRST_NODE (~((graph)-1 >> 1))
#define FIRST_NODES(n) (~((graph)-1 >> (n)))
#define FIRST_TWO_NODES FIRST_NODES(2)
#define NTH_NODE(n) (FIRST_NODE >> (n))

#define PRUNE prunelaman
// #define OUTPROC countgraphs

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

static nauty_counter total_number_of_graphs = 0;

void countgraphs(FILE *f, graph *g, int n)
{
    ++total_number_of_graphs;
    /* report on every 2^30 ≈ 10^9 graph ≈ every hour */
    if ((total_number_of_graphs & (1 << 30) - 1) == 0)
    {
        fprintf(stderr, ">A " COUNTER_FMT " graphs generated\n",
                total_number_of_graphs);
        fflush(stderr);
    }
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
        mask = FIRST_NODES(k - 1) | NTH_NODE(n - 1);
        l = 0;
        for (i = 0; i < k - 1; ++i)
        {
            nodeinds[i] = i;
            l += POPCOUNT(g[i] & mask);
        }
        nodeinds[k - 1] = n - 1;
        l += POPCOUNT(g[n - 1] & mask);
        l = l / 2;

        subgraphsleft = TRUE;
        while (subgraphsleft)
        {
            /* subgraph is overdetermined => not minimal */
            if (l > EMBED_DIM * k - EMBED_DOF)
                return TRUE;

            /* go to next subgraph */
            for (i = k - 2; i >= 0; --i)
            {
                if (nodeinds[i] < n + i - k)
                {
                    l -= POPCOUNT(g[nodeinds[i]] & mask);
                    mask ^= FIRST_TWO_NODES >> nodeinds[i];
                    ++nodeinds[i];
                    l += POPCOUNT(g[nodeinds[i]] & mask);

                    for (j = i + 1; j < k - 1; ++j)
                    {
                        l -= POPCOUNT(g[nodeinds[j]] & mask);
                        mask &= ~NTH_NODE(nodeinds[j]);
                        nodeinds[j] = nodeinds[i] + j - i;
                        mask |= NTH_NODE(nodeinds[j]);
                        l += POPCOUNT(g[nodeinds[j]] & mask);
                    }
                    break;
                }
                else
                {
                    /* wrap, continue and increase next index */
                    if (i == 0)
                        subgraphsleft = FALSE;
                }
            }
        }
    }
    return FALSE;
}
