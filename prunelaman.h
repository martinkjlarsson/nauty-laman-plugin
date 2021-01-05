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
#define PLUGIN_INIT                                                                                   \
    if (tightkd < 0)                                                                                  \
    {                                                                                                 \
        tightkn = -tightkn;                                                                           \
        tightkd = -tightkd;                                                                           \
    }                                                                                                 \
    if (tightld < 0)                                                                                  \
    {                                                                                                 \
        tightln = -tightln;                                                                           \
        tightld = -tightld;                                                                           \
    }                                                                                                 \
    if (tightkn == tightkd)                                                                           \
        tightkd = 1;                                                                                  \
    else if (tightkd == 0)                                                                            \
        gt_abort(">E geng: -K has to be an number\n");                                                \
    if (tightln == tightld)                                                                           \
        tightld = 1;                                                                                  \
    else if (tightld == 0)                                                                            \
        gt_abort(">E geng: -L has to be an number\n");                                                \
    if (!gotL)                                                                                        \
    {                                                                                                 \
        tightln = tightkn * (tightkn + tightkd) / 2;                                                  \
        tightld = tightkd * tightkd;                                                                  \
    }                                                                                                 \
    if (henneberg1)                                                                                   \
    {                                                                                                 \
        prune = prunehenneberg1;                                                                      \
        if (tightkd != 1)                                                                             \
            gt_abort(">E geng: -K has to be an integer\n");                                           \
        if (gotd || gote || gotL)                                                                     \
            gt_abort(">E geng: -deK are incompatible with -H\n");                                     \
    }                                                                                                 \
    else if (gotK)                                                                                    \
    {                                                                                                 \
        prune = prunetight;                                                                           \
    }                                                                                                 \
    else                                                                                              \
    {                                                                                                 \
        prune = nopruning;                                                                            \
        if (gotL)                                                                                     \
            gt_abort(">E geng: -K is required when providing -L\n");                                  \
    }                                                                                                 \
    if (henneberg1 || gotK)                                                                           \
    {                                                                                                 \
        if (maxn > 1)                                                                                 \
        {                                                                                             \
            int maxtightedges = (tightkn * tightld * maxn - tightln * tightkd) / (tightkd * tightld); \
            if (!gote)                                                                                \
                geng_mine = geng_maxe = mine = maxe = maxtightedges;                                  \
            else if (maxe > maxtightedges)                                                            \
                geng_maxe = maxe = maxtightedges;                                                     \
        }                                                                                             \
        if (!gotd && !gote && maxn > tightkn / tightkd)                                               \
            geng_mindeg = mindeg = tightkn / tightkd;                                                 \
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

/* Generates the next combination of k items from n possible ones, i.e., the
 * next k-subset of an n-set. If A is initialized with the items 0..k-1,
 * repeatedly calling the function will generate all possible combinations
 * until it circles back to 0..k-1, at which point FALSE is return. The
 * combinations are constructed in such a way that only one item is removed and
 * replaced every call (a so called combinatorial Gray code).
 *
 * See Nijenhuis, Albert, and Herbert S. Wilf. Combinatorial algorithms: for
 * computers and calculators. Elsevier, 2014. for the original FORTRAN code.
 *
 * Arguments:
 * n - the number of items to choose from.
 * k - the number of items to choose.
 * A - the current combination which will be updated to contain the next one.
 * in - will be updated to contain the item which was added to A.
 * out - will be updated to contain the item which was removed from A.
 *
 * Returns:
 * FALSE if the returned combination in A is 0..k-1 and TRUE otherwise.
 */
inline boolean nxksrd(int n, int k, int *restrict A, int *restrict in, int *restrict out)
{
    int j, m;

    j = 0;
    if (k & 1)
    {
        // 100
        m = j < k - 1 ? A[j + 1] - 1 : n - 1;
        if (m != A[j])
        {
            *out = A[j];
            A[j] = A[j] + 1;
            *in = A[j];
            if (j != 0) // else goto 200
            {
                A[j - 1] = *out;
                *out = j - 1;
            }
            return TRUE; // goto 200
        }
        j++;
    }

    while (j < k)
    {
        // 30
        if (A[j] != j) // else goto 100
        {
            *out = A[j];
            A[j] = A[j] - 1;
            *in = A[j];
            if (j != 0)
            {
                *in = j - 1;
                A[j - 1] = *in;
            }
            return TRUE; // goto 200
        }
        j++;

        // 100
        m = j < k - 1 ? A[j + 1] - 1 : n - 1;
        if (m != A[j])
        {
            *out = A[j];
            A[j] = A[j] + 1;
            *in = A[j];
            if (j != 0) // else goto 200
            {
                A[j - 1] = *out;
                *out = j - 1;
            }
            return TRUE; // goto 200
        }
        j++;
    }

    // 40
    A[k - 1] = k - 1;
    *in = k - 1;
    *out = n - 1;
    return FALSE;
}

/* dummy function when no pruning is applied */
int nopruning(graph *g, int n, int maxn)
{
    return FALSE;
}

/* remove graphs that are not (k,l)-sparse */
int prunetight(graph *g, int n, int maxn)
{
    int i, k, l, m;
    int nodeinds[MAXN];
    int in, out;
    setword mask;

    /* subgraphs with n <= tightkn/tightkd+1 cannot be overdetermined */
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

    /* Go through all subgraphs verifying sparsity. geng constructs graphs by
     * successively adding more nodes. Therefore, we only need to check the
     * subgraphs containing the new last node. The other subgraphs have been
     * checked in previous steps. The first subgraph consists of all nodes
     * except the second to last one. */
    l = m - POPCOUNT(g[n - 2]);
    mask = ALLMASK(n) & ~NTH_NODE(n - 2);
    for (i = 0; i < n - 1; ++i)
        nodeinds[i] = i;

    /* go through all k-vertex subgraphs */
    for (k = n - 1; k > tightkn / tightkd + 1; --k)
    {
        if (TOO_MANY_EDGES(k, l))
            return TRUE;

        while (nxksrd(n - 1, k - 1, nodeinds, &in, &out))
        {
            l -= POPCOUNT(g[out] & mask);
            mask ^= NTH_NODE(out);
            mask ^= NTH_NODE(in);
            l += POPCOUNT(g[in] & mask);

            if (TOO_MANY_EDGES(k, l))
                return TRUE;
        }
        /* nodeinds == 0..k-2, in == k-2, out == n-2 */
        l -= POPCOUNT(g[out] & mask);
        mask ^= NTH_NODE(out);
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
