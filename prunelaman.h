/* Copyright (c) 2020 Martin Larsson */

/* Add this flag when compiling geng: -D'PLUGIN="prunelaman.h"' */

#define NTH_NODE(n) (bit[n]) /* Apparently lookup is faster than bitshift. */

/* Uncomment to enable status reports to stderr. This will disable any other
 * output options -uyngs. */
// #define OUTPROC countgraphs

/* Pruning function. */
#define PRUNE (*prune)

/* Parse plugin arguments. */
#define PLUGIN_SWITCHES \
    else SWINT('k', gotk, tightk, "geng -k") else SWINT('K', gotK, tightK, "geng -K") else SWBOOLEAN('L', henneberg1)

/* Note: PLUGIN_INIT happens after validation of the input arguments in geng.c.
 * Beware of illegal argument combinations. */
#define PLUGIN_INIT                                                       \
    if (henneberg1)                                                       \
    {                                                                     \
        prune = prunehenneberg1;                                          \
        if (!gotk)                                                        \
            tightk = 2;                                                   \
        if (gotd || gote || gotK)                                         \
            gt_abort(">E geng: -deK are incompatible with -L\n");         \
        tightK = tightk * (tightk + 1) / 2;                               \
    }                                                                     \
    else if (gotk)                                                        \
    {                                                                     \
        prune = prunetight;                                               \
        if (!gotK)                                                        \
            tightK = tightk * (tightk + 1) / 2;                           \
    }                                                                     \
    else                                                                  \
    {                                                                     \
        prune = nopruning;                                                \
        if (gotK)                                                         \
            gt_abort(">E geng: -k is required when providing -K\n");      \
    }                                                                     \
    if (henneberg1 || gotk)                                               \
    {                                                                     \
        if (!gote && maxn > 1)                                            \
            geng_mine = geng_maxe = mine = maxe = tightk * maxn - tightK; \
        if (!gotd && maxn > tightk)                                       \
            geng_mindeg = mindeg = tightk;                                \
    }

static int (*prune)(graph *, int, int);
static boolean gotk = FALSE;
static boolean gotK = FALSE;
static int tightk; /* Specifies k for (k,l)-tight graphs. */
static int tightK; /* Specifies l for (k,l)-tight graphs. */
static boolean henneberg1 = FALSE;
static nauty_counter total_number_of_graphs = 0;

/* If OUTPROC is defined as above, this gets called whenever a new graph has
 * been generated. Instead of outputting to file this procedure simply counts
 * the graphs and provides a status report every now and then. */
void countgraphs(FILE *f, graph *g, int n)
{
    ++total_number_of_graphs;
    /* report number of graphs generated approximately every hour */
    if ((total_number_of_graphs & (1 << 40 - n) - 1) == 0)
    {
        fprintf(stderr, ">A " COUNTER_FMT " graphs generated\n",
                total_number_of_graphs);
        fflush(stderr);
    }
}

int nopruning(graph *g, int n, int maxn)
{
    return FALSE;
}

/* remove graphs that are not (k,l)-sparse */
int prunetight(graph *g, int n, int maxn)
{
    int i, j, k, l, m;
    boolean subgraphsleft;
    int nodeinds[MAXN];
    setword mask;

    /* subgraphs with n <= tightk+1 cannot be overdetermined
     * graph underdeterminedness is prevented using mine and maxe */
    if (n <= tightk + 1)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not minimal */
    if (m > tightk * n - tightK)
        return TRUE;

    /* Go through all subgraphs to make sure m <= tightk*n-tightK.
     * geng constructs graphs by succesively adding more nodes. Therefore,
     * we only need to check the subgraphs containing the new node. The other
     * subgraphs have been checked in previous steps. */
    for (k = n - 1; k > tightk + 1; --k)
    {
        /* init subgraph with the k-1 first nodes and the new node */
        mask = ALLMASK(k - 1) | NTH_NODE(n - 1);
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
            if (l > tightk * k - tightK)
                return TRUE;

            /* go to next subgraph */
            subgraphsleft = FALSE;
            for (i = k - 2; i >= 0; --i)
            {
                if (nodeinds[i] < n + i - k)
                {
                    l -= POPCOUNT(g[nodeinds[i]] & mask);
                    mask ^= ALLMASK(2) >> nodeinds[i];
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
                    subgraphsleft = TRUE;
                    break;
                }
            }
        }
    }
    return FALSE;
}

/* remove graphs that cannot be constructed using Henneberg type I moves */
int prunehenneberg1(graph *g, int n, int maxn)
{
    int i, m;
    setword mask, tovisit;

    /* subgraphs with n <= tightk+1 cannot be overdetermined
     * graph underdeterminedness is prevented using mine and maxe */
    if (n <= tightk + 1)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not minimal */
    if (m > tightk * n - tightK)
        return TRUE;

    /* we are done with subgraphs */
    if (n != maxn)
        return FALSE;

    /* deconstruct graph by reversing Henneberg type I moves */
    mask = ALLMASK(n);
    tovisit = ALLMASK(n);
    while (tovisit)
    {
        i = FIRSTBITNZ(tovisit);
        tovisit &= ~NTH_NODE(i);
        if (POPCOUNT(g[i] & mask) == tightk)
        {
            tovisit |= g[i] & mask;
            mask &= ~NTH_NODE(i);
        }
    }
    return POPCOUNT(mask) > tightk;
}
