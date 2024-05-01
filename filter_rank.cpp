/*
    Filter graphs based on the rank of the rigidity matrix for random realizations.
    See help text for details: filter_rank -h

    This program uses Eigen (https://eigen.tuxfamily.org/). Provide the appropriate
    include flags when compiling.

    Examples:
        Count all rigid (3,6)-tight graphs of order 8:
            ./gensparseg 8 -K3 | ./filter_rank -u
            >A Laman plugin -K3L6N4
            >A ./gensparseg -d3D7 n=8 e=18
            >Z 375 graphs generated in 0.00 sec
            >Z 374 graphs passed rank filter in 0.01 sec

        Find all flexible (3,6)-tight graphs of order 9:
            ./gensparseg 9 -K3 | ./filter_rank -cp
            >A Laman plugin -K3L6N4
            >A ./gensparseg -d3D8 n=9 e=21
            >Z 11495 graphs generated in 0.01 sec
            HCQRV~} 1
            HCQrU~} 1
            HCXmd}} 1
            HCdbF~} 1
            HCdbNz} 1
            HCdbM~} 1
            HQhTVz{ 1
            HQhTVzu 1
            >Z 8 graphs passed rank filter in 0.12 sec

        Find all flexible (3,6)-tight graphs of order 11 with two degrees of freedom:
            ./gensparseg 11 -K3 | ./filter_rank 3 2
            >A Laman plugin -K3L6N4
            >A ./gensparseg -d3D10 n=11 e=27
            JCOcaOc~~~?
            >Z 48185341 graphs generated in 208.37 sec
            >Z 1 graphs passed rank filter in 710.35 sec
*/

#include <exception>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include "Eigen/Dense"
#include "Eigen/SVD"
#include "Eigen/QR"

using namespace std;
using namespace Eigen;

const string help_text =
    "Usage: filter_rank [dim [dof [trials]]] [-cpu]\n\n"
    "Filter graphs based on the rank of the rigidity matrix for random realizations.\n"
    "With the default arguments, the filter will keep all rigid graphs in 3D.\n\n"
    "    dim     : the dimension of the space (default 3).\n"
    "    dof     : the excessive degrees of freedom in the graph beyond the trivial\n"
    "              motions (default 0). A graph passes the filter if the rigidity\n"
    "              matrix has the rank dim * n - dim * (dim + 1) / 2 - dof, where n\n"
    "              is the number of nodes.\n"
    "    trials  : the number of realizations to check (default 1). Majority voting\n"
    "              is used to determine whether or not a graph passed the filter.\n"
    "    -c      : inverts the filter and returns the graphs that do not have the\n"
    "              desired rank. If dof=0, this will result in all flexible graphs.\n"
    "    -p      : outputs the excessive degrees of freedom along with the graphs.\n"
    "    -u      : suppresses the output and only counts the graphs.";

struct Edge
{
    int s;
    int d;

    Edge(int s, int d) : s(s), d(d){};
};

struct Graph
{
    int n;
    vector<Edge> edge_list;

    Graph(int n) : n(n), edge_list(){};
};

// See http://users.cecs.anu.edu.au/~bdm/data/formats.txt
Graph parse_graph6(string graph6)
{
    if (graph6.empty())
        throw std::invalid_argument("Empty string is not a valid graph6 graph.");
    if (graph6[0] == 126)
        throw std::invalid_argument("Support for graphs with more than 62 vertices is not implemented.");

    int n = graph6[0] - 63;
    Graph g(n);

    int bit_index = -1;
    int char_index = 1;
    int data = 0;
    for (int j = 1; j < n; j++)
    {
        for (int i = 0; i < j; i++)
        {
            if (bit_index < 0)
            {
                // No bounds check on graph6. We assume correctly formatted string.
                data = graph6[char_index] - 63;
                char_index++;
                bit_index = 5;
            }
            if (data & (1 << bit_index))
            {
                // Edge (i,j) exists.
                g.edge_list.emplace_back(i, j);
            }
            bit_index--;
        }
    }

    return g;
}

int rigidity_rank(const Graph &g, int dim)
{
    int n = g.n;
    int m = g.edge_list.size();

    MatrixXd p = MatrixXd::Random(dim, n);
    MatrixXd R = MatrixXd::Zero(m, dim * n);

    for (int i = 0; i < m; i++)
    {
        Edge e = g.edge_list[i];
        VectorXd v = p.col(e.s) - p.col(e.d);
        R.block<1, Dynamic>(i, dim * e.s, 1, dim) = v;
        R.block<1, Dynamic>(i, dim * e.d, 1, dim) = -v;
    }

    ColPivHouseholderQR<MatrixXd> decomp(R);
    return decomp.rank();
}

bool parse_int(string s, int *i)
{
    try
    {
        *i = stoi(s);
        return true;
    }
    catch (const invalid_argument &e)
    {
        return false;
    }
    catch (const out_of_range &e)
    {
        return false;
    }
}

bool parse_arguments(int argc, const char *argv[], vector<int *> pos_args, vector<pair<char, bool *>> flag_args)
{
    int pos = 0;
    for (int i = 1; i < argc; i++)
    {
        string s = argv[i];
        if (pos < pos_args.size() && parse_int(s, pos_args[pos]))
        {
            pos++;
            continue;
        }

        if (s[0] != '-')
            return false;

        for (auto ch : s.substr(1))
        {
            bool found_flag = false;
            for (auto flag : flag_args)
            {
                if (ch == flag.first)
                {
                    *flag.second = true;
                    found_flag = true;
                    break;
                }
            }
            if (!found_flag)
                return false;
        }
    }
    return true;
}

int main(int argc, const char *argv[])
{
    int dim = 3;
    int dof = 0;
    int trials = 1;
    bool complement = false;
    bool print_dof = false;
    bool nooutput = false;

    vector<int *> pos_args = {&dim, &dof, &trials};
    vector<pair<char, bool *>> flag_args = {
        pair<char, bool *>('c', &complement),
        pair<char, bool *>('p', &print_dof),
        pair<char, bool *>('u', &nooutput)};

    if (!parse_arguments(argc, argv, pos_args, flag_args))
    {
        cout << help_text << endl;
        return 1;
    }
    if (dim < 1)
    {
        cout << ">E filter_rank: dim has to be positive" << endl;
        return 1;
    }
    if (dof < 0)
    {
        cout << ">E filter_rank: dof cannot be negative" << endl;
        return 1;
    }
    if (trials < 1 || (trials % 2 == 0))
    {
        cout << ">E filter_rank: trials has to be positive and odd" << endl;
        return 1;
    }

    int gauge_freedom = dim * (dim + 1) / 2;

    int count = 0;
    string graph6;
    clock_t c_start = clock();
    vector<int> excessive_dof(trials);
    while (getline(cin, graph6))
    {
        Graph g = parse_graph6(graph6);
        int rigid_rank = dim * g.n - gauge_freedom;
        int passes = 0;
        for (int i = 0; i < trials; i++)
        {
            int edof = rigid_rank - rigidity_rank(g, dim);
            excessive_dof[i] = edof;
            if ((edof == dof) != complement)
                passes++;
        }
        // Warn if majority voting was necessary. If this happens often trials might need to be increased.
        if (passes != 0 && passes != trials)
            cerr << ">Z majority voting was required " << passes << '/' << trials << endl;
        if (passes > trials / 2)
        {
            count++;
            if (!nooutput)
            {
                cout << graph6;
                if (print_dof)
                {
                    for (int edof : excessive_dof)
                        cout << ' ' << edof;
                }
                cout << '\n';
            }
        }
    }
    clock_t c_end = clock();
    double time = double(c_end - c_start) / CLOCKS_PER_SEC;
    cerr.precision(2);
    cerr << ">Z " << count << " graphs passed rank filter in " << fixed << time << " sec" << endl;
    return 0;
}
