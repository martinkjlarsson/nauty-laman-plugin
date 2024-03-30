/* Copyright (c) 2020 Martin Larsson */

/* Using rationals for (k,l) comes with a small overhead. Define this macro to use integers for (k,l). */
// #define INT_KL

#define NTH_NODE(n) (bit[n]) /* Apparently lookup is faster than bitshift. */

/* Comment out if __builtin_ctz is missing. */
#define HAVE_CTZ

#if defined(HAVE_CTZ)
#define CTZ(x) __builtin_ctz(x)
#else
#if MAXN > 32
#error Manual CTZ implementation only supports MAXN <= 32.
#endif
static const int MultiplyDeBruijnBitPosition[32] =
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
#define CTZ(x) MultiplyDeBruijnBitPosition[(((unsigned)(x) & -(unsigned)(x)) * 0x077CB531U) >> 27]
// #include <string.h>
// #define CTZ(x) (ffs(x) - 1) // This is ridiculously slow...
#endif

/* Uncomment to enable status reports to stderr. This will disable any other
 * output options -uyngs. */
// #define OUTPROC countgraphs

/* Pruning function. */
#define PRUNE (*prune)

/* Parse plugin arguments. */
#ifdef INT_KL
#define TOO_MANY_EDGES(n, m) ((m) > tightkn * (n)-tightln)
#define PLUGIN_SWITCHES else SWINT('K', gotK, tightkn, "gensparseg -K") else SWINT('L', gotL, tightln, "gensparseg -L") else SWBOOLEAN('H', henneberg1) else SWINT('N', gotN, minn, "gensparseg -N")
#define PRINT_LAMAN_MESSAGE fprintf(stderr, ">A Laman plugin -K%dL%dN%d\n", tightkn, tightln, minn);
#else
#define TOO_MANY_EDGES(n, m) (tightkd * tightld * (m) > tightkn * tightld * (n)-tightln * tightkd)
#define PLUGIN_SWITCHES else SWRANGE('K', "/", gotK, tightkn, tightkd, "gensparseg -K") else SWRANGE('L', "/", gotL, tightln, tightld, "gensparseg -L") else SWBOOLEAN('H', henneberg1) else SWINT('N', gotN, minn, "gensparseg -N")

#define PRINT_LAMAN_MESSAGE                                                        \
    if (tightkd == 1 && tightld == 1)                                              \
        fprintf(stderr, ">A Laman plugin -K%ldL%ldN%d\n", tightkn, tightln, minn); \
    else                                                                           \
        fprintf(stderr, ">A Laman plugin -K%ld/%ldL%ld/%ldN%d\n", tightkn, tightkd, tightln, tightld, minn);
#endif

/* Note: PLUGIN_INIT happens after validation of the input arguments in geng.c.
 * Beware of illegal argument combinations. */
#define PLUGIN_INIT                                                                                       \
    if (tightkn == tightkd)                                                                               \
        tightkd = 1;                                                                                      \
    else if (tightkd == 0)                                                                                \
        gt_abort(">E gensparseg: -K has to be a number\n");                                               \
    if (tightln == tightld)                                                                               \
        tightld = 1;                                                                                      \
    else if (tightld == 0)                                                                                \
        gt_abort(">E gensparseg: -L has to be a number\n");                                               \
    if (tightkd < 0)                                                                                      \
    {                                                                                                     \
        tightkn = -tightkn;                                                                               \
        tightkd = -tightkd;                                                                               \
    }                                                                                                     \
    if (tightld < 0)                                                                                      \
    {                                                                                                     \
        tightln = -tightln;                                                                               \
        tightld = -tightld;                                                                               \
    }                                                                                                     \
    if (!gotL)                                                                                            \
    {                                                                                                     \
        if (tightkd & 1)                                                                                  \
        {                                                                                                 \
            tightln = tightkn * (tightkn + tightkd) / 2;                                                  \
            tightld = tightkd * tightkd;                                                                  \
        }                                                                                                 \
        else                                                                                              \
        {                                                                                                 \
            tightln = tightkn * (tightkn + tightkd);                                                      \
            tightld = 2 * tightkd * tightkd;                                                              \
        }                                                                                                 \
    }                                                                                                     \
    if (gotN)                                                                                             \
    {                                                                                                     \
        if (minn < 2)                                                                                     \
            gt_abort(">E gensparseg: -N has to be at least 2\n");                                         \
    }                                                                                                     \
    else                                                                                                  \
    {                                                                                                     \
        minn = tightkn / tightkd;                                                                         \
        if (minn < 2)                                                                                     \
            minn = 2;                                                                                     \
        while (!TOO_MANY_EDGES(minn + 1, minn * (minn + 1) / 2))                                          \
            minn++;                                                                                       \
    }                                                                                                     \
    if (henneberg1)                                                                                       \
    {                                                                                                     \
        prune = prunehenneberg1;                                                                          \
        if (tightkd != 1)                                                                                 \
            gt_abort(">E gensparseg: -K has to be an integer\n");                                         \
        if (gotd || gote || gotL)                                                                         \
            gt_abort(">E gensparseg: -deL are incompatible with -H\n");                                   \
    }                                                                                                     \
    else if (gotK)                                                                                        \
    {                                                                                                     \
        if (tightkd == 1 && tightld == 1 && tightln >= 0 && tightln < 2 * tightkn && minn <= tightkn + 1) \
            prune = prunetightpebble;                                                                     \
        else if (tightkn < 2 * tightkd)                                                                   \
            prune = prunetightcomb;                                                                       \
        else                                                                                              \
            prune = prunetightgray;                                                                       \
    }                                                                                                     \
    else                                                                                                  \
    {                                                                                                     \
        prune = nopruning;                                                                                \
        if (gotL)                                                                                         \
            gt_abort(">E gensparseg: -K is required when providing -L\n");                                \
    }                                                                                                     \
    if (henneberg1 || gotK)                                                                               \
    {                                                                                                     \
        int maxtightedges = (tightkn * tightld * maxn - tightln * tightkd) / (tightkd * tightld);         \
        if (maxn <= minn)                                                                                 \
            maxtightedges = maxn * (maxn - 1) / 2;                                                        \
        if (!gote)                                                                                        \
            geng_mine = geng_maxe = mine = maxe = maxtightedges;                                          \
        else if (maxe > maxtightedges)                                                                    \
            geng_maxe = maxe = maxtightedges;                                                             \
        if (!gotd && !gote && maxn > tightkn / tightkd)                                                   \
            geng_mindeg = mindeg = tightkn / tightkd;                                                     \
        if (!quiet)                                                                                       \
            PRINT_LAMAN_MESSAGE                                                                           \
    }

static int (*prune)(graph *, int, int);
static boolean gotK = FALSE;
static boolean gotL = FALSE;
static boolean gotN = FALSE;
#ifdef INT_KL
static int tightkn = 2; /* Specifies k for (k,l)-tight graphs. */
static int tightkd = 1;
static int tightln = 3; /* Specifies l for (k,l)-tight graphs. */
static int tightld = 1;
#else
static long tightkn = 2; /* Specifies k for (k,l)-tight graphs. */
static long tightkd = 1;
static long tightln = 3; /* Specifies l for (k,l)-tight graphs. */
static long tightld = 1;
#endif
static int minn = 2;
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

int find_pebble(graph *d, int *pebbles, int n, setword *tovisit, int i)
{
    int j;
    while (d[i] & *tovisit)
    {
        j = FIRSTBITNZ(d[i] & *tovisit);
        *tovisit &= ~NTH_NODE(j);

        if (pebbles[j] > 0)
        {
            pebbles[j]--;
            d[i] &= ~NTH_NODE(j);
            d[j] |= NTH_NODE(i);
            return TRUE;
        }
        else if (find_pebble(d, pebbles, n, tovisit, j))
        {
            d[i] &= ~NTH_NODE(j);
            d[j] |= NTH_NODE(i);
            return TRUE;
        }
    }
    return FALSE;
}

/* Determine whether the provided graph on n vertices is (k,l)-tight, (k,l)-sparse, or
 * overconstrained.

 * See Lee and Streinu (2008) Pebble game algorithms and sparse graphs
 *
 * Returns:
 * <0 if the graph is overconstrained
 * 0 if the graph is (k,l)-tight
 * >0 if the graph is (k,l)-sparse
 */
int pebblegame(graph *g, int n, int k, int l)
{
    int i, j, total, needed;
    int pebbles[MAXN];
    setword tovisit, inittovisit;
    graph d[MAXN] = {0};

    for (i = 0; i < n; ++i)
        pebbles[i] = k;

    for (i = 0; i < n; ++i)
    {
        for (j = 0; j < i; ++j)
        {
            if ((g[i] & NTH_NODE(j)) == 0)
                continue;

            // (i,j) is an edge.
            needed = l + 1 - pebbles[i] - pebbles[j];
            inittovisit = ALLMASK(n) & ~NTH_NODE(i) & ~NTH_NODE(j);
            tovisit = inittovisit;
            while (needed > 0 && pebbles[i] < k && find_pebble(d, pebbles, n, &tovisit, i))
            {
                needed--;
                pebbles[i]++;
                tovisit = inittovisit;
            }
            tovisit = inittovisit;
            while (needed > 0 && pebbles[j] < k && find_pebble(d, pebbles, n, &tovisit, j))
            {
                needed--;
                pebbles[j]++;
                tovisit = inittovisit;
            }
            if (needed > 0)
                return -1;

            if (pebbles[i] > pebbles[j])
            {
                pebbles[i]--;
                d[i] |= NTH_NODE(j);
            }
            else
            {
                pebbles[j]--;
                d[j] |= NTH_NODE(i);
            }
        }
    }

    total = 0;
    for (i = 0; i < n; ++i)
        total += pebbles[i];
    return total - l;
}

/* dummy function when no pruning is applied */
int nopruning(graph *g, int n, int maxn)
{
    return FALSE;
}

/* remove graphs that are not (k,l)-sparse
 * seems to have better performance than prunetightgray for k < 2 */
int prunetightcomb(graph *g, int n, int maxn)
{
    int i, k, l, m;
    int nodeinds[MAXN];
    int in, out;
    setword mask;

    /* small graphs are considered sparse */
    if (n <= minn)
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
    for (k = n - 1; k > minn; --k)
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

/* remove graphs that are not (k,l)-sparse
 * seems to have better performance than prunetightcomb for k >= 2 */
int prunetightgray(graph *g, int n, int maxn)
{
    int i, j, m, k, l, degree;
    setword mask;

    /* small graphs are considered sparse */
    if (n <= minn)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not sparse */
    if (TOO_MANY_EDGES(n, m))
        return TRUE;

    /* Go through all subgraphs verifying sparsity. We use the Gray code binary
     * representation of i as a mask for which nodes are included in the
     * subgraph. This way, in every iteration, we either add or remove a single
     * node to the previous subgraph. */
    k = 1;
    l = 0;
    mask = NTH_NODE(n - 1); /* always include the new node */
    for (i = 1; i < (1 << n - 1); ++i)
    {
        j = CTZ(i);
        mask ^= NTH_NODE(j); /* add or remove node */
        degree = POPCOUNT(g[j] & mask);
        l += mask & NTH_NODE(j) ? degree : -degree;
        k += mask & NTH_NODE(j) ? 1 : -1;

        if (k > minn && TOO_MANY_EDGES(k, l))
            return TRUE;
    }
    return FALSE;
}

/* remove graphs that are not (k,l)-sparse
 * performs much better than the other methods for integer k and l such that 0 <= l < 2k */
int prunetightpebble(graph *g, int n, int maxn)
{
    int i, j, m, k, l, degree;
    setword mask;

    /* small graphs are considered sparse */
    if (n <= minn)
        return FALSE;

    /* find number of edges */
    m = 0;
    for (i = 0; i < n; ++i)
        m += POPCOUNT(g[i]);
    m = m / 2;

    /* subgraph is overdetermined => not sparse */
    if (m > tightkn * n - tightln)
        return TRUE;

    return pebblegame(g, n, tightkn, tightln) < 0;
}

/* remove graphs that cannot be constructed using Henneberg type I moves */
int prunehenneberg1(graph *g, int n, int maxn)
{
    int i, m;
    setword mask, tovisit;

    /* small graphs are considered sparse */
    if (n <= minn)
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
