/* Copyright (c) 2020 Martin Larsson */

/* Add this flag when compiling geng: -D'PLUGIN="prunelaman.h"' */

/* Using rationals for (k,l) comes with a small overhead. Define this macro to use integers for (k,l). */
// #define INT_KL

#define NTH_NODE(n) (bit[n]) /* Apparently lookup is faster than bitshift. */

/* Uncomment to enable status reports to stderr. This will disable any other
 * output options -uyngs. */
// #define OUTPROC countgraphs

/* Pruning function. */
#define PRUNE (*prune)

/* Parse plugin arguments. */
#ifdef INT_KL
#define TOO_MANY_EDGES(n, m) ((m) > tightkn * (n)-tightln)
#define PLUGIN_SWITCHES else SWLONG('K', gotK, tightkn, "geng -K") else SWLONG('L', gotL, tightln, "geng -L") else SWBOOLEAN('H', henneberg1)
#else
#define TOO_MANY_EDGES(n, m) (tightkd * tightld * (m) > tightkn * tightld * (n)-tightln * tightkd)
#define PLUGIN_SWITCHES else SWRANGE('K', "/", gotK, tightkn, tightkd, "geng -K") else SWRANGE('L', "/", gotL, tightln, tightld, "geng -L") else SWBOOLEAN('H', henneberg1)
#endif

/* Note: PLUGIN_INIT happens after validation of the input arguments in geng.c.
 * Beware of illegal argument combinations. */
#define PLUGIN_INIT                                                                   \
    if (tightkn == tightkd)                                                           \
        tightkd = 1;                                                                  \
    else if (tightkd == 0)                                                            \
        gt_abort(">E geng: -K has to be an number\n");                                \
    if (tightln == tightld)                                                           \
        tightld = 1;                                                                  \
    else if (tightld == 0)                                                            \
        gt_abort(">E geng: -L has to be an number\n");                                \
    if (!gotL)                                                                        \
    {                                                                                 \
        tightln = tightkn * (tightkn + tightkd) / 2;                                  \
        tightld = tightkd * tightkd;                                                  \
    }                                                                                 \
    if (henneberg1)                                                                   \
    {                                                                                 \
        prune = prunehenneberg1;                                                      \
        if (tightkd != 1)                                                             \
            gt_abort(">E geng: -K has to be an integer\n");                           \
        if (gotd || gote || gotL)                                                     \
            gt_abort(">E geng: -deK are incompatible with -H\n");                     \
    }                                                                                 \
    else if (gotK)                                                                    \
    {                                                                                 \
        prune = prunetight;                                                           \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        prune = nopruning;                                                            \
        if (gotL)                                                                     \
            gt_abort(">E geng: -K is required when providing -L\n");                  \
    }                                                                                 \
    if (henneberg1 || gotK)                                                           \
    {                                                                                 \
        if (!gote && maxn > 1)                                                        \
            geng_mine = geng_maxe = mine = maxe =                                     \
                (tightkn * tightld * maxn - tightln * tightkd) / (tightkd * tightld); \
        if (!gotd && maxn > tightkn / tightkd)                                        \
            geng_mindeg = mindeg = tightkn / tightkd;                                 \
    }

static int (*prune)(graph *, int, int);
static boolean gotK = FALSE;
static boolean gotL = FALSE;
static long tightkn = 2; /* Specifies k for (k,l)-tight graphs. */
static long tightkd = 1;
static long tightln = 3; /* Specifies l for (k,l)-tight graphs. */
static long tightld = 1;
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

    /* subgraphs with n <= tightkn/tightkd+1 cannot be overdetermined
     * graph underdeterminedness is prevented using mine and maxe */
    if (n <= tightkn / tightkd + 1)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not sparse */
    if (TOO_MANY_EDGES(n, m))
        return TRUE;

    /* Go through all subgraphs to make sure they are sparse.
     * geng constructs graphs by succesively adding more nodes. Therefore,
     * we only need to check the subgraphs containing the new node. The other
     * subgraphs have been checked in previous steps. */
    for (k = n - 1; k > tightkn / tightkd + 1; --k)
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
            /* subgraph is overdetermined => not sparse */
            if (TOO_MANY_EDGES(k, l))
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

    /* subgraphs with n <= tightkn+1 cannot be overdetermined
     * graph underdeterminedness is prevented using mine and maxe */
    if (n <= tightkn + 1)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not sparse */
    if (m > tightkn * n - tightln)
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
        if (POPCOUNT(g[i] & mask) == tightkn)
        {
            tovisit |= g[i] & mask;
            mask &= ~NTH_NODE(i);
        }
    }
    return POPCOUNT(mask) > tightkn;
}
