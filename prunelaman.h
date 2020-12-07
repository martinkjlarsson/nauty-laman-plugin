/* Copyright (c) 2020 Martin Larsson */

/* Add this flag when compiling geng: -D'PLUGIN="prunelaman.h"' */

/* Note: This code, i.e., the Laman conditions, only works propertly for
 * embeddings in the plane. For higher dimensions this code will also generate
 * graphs that are not minimally rigid, e.g., the double-banana graph in 3D. */
#define EMBED_DIM 2
#define EMBED_DOF (EMBED_DIM * (EMBED_DIM + 1) / 2)
#define FIRST_NODE (~((graph)-1 >> 1))
#define FIRST_NODES(n) (~((graph)-1 >> (n)))
#define NTH_NODE(n) (FIRST_NODE >> (n))

#define PRUNE prunelaman

/* Uncomment to enable status reports to stderr. This will disable any other
 * output options -uyngs. */
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

/* If OUTPROC is defined as above, this gets called whenever a new graph has
 * been generated. Instead of outputting to file this procedure simply counts
 * the graphs and provides a status report every now and then. */
void countgraphs(FILE *f, graph *g, int n)
{
    ++total_number_of_graphs;
    /* report on every 2^28 graph ≈ every hour */
    if ((total_number_of_graphs & (1 << 28) - 1) == 0)
    {
        fprintf(stderr, ">A " COUNTER_FMT " graphs generated\n",
                total_number_of_graphs);
        fflush(stderr);
    }
}

/* Pruning step that gets called whenever a new graph or subgraph is generated. */
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
                    mask ^= FIRST_NODES(2) >> nodeinds[i];
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
                else if (i == 0)
                {
                    subgraphsleft = FALSE;
                }
            }
        }
    }
    return FALSE;
}
