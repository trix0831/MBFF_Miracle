#include <unordered_map>
#include <vector>
#include <set>
using namespace std;
#include "Instance.h"

class Graph
{
private:
    unordered_map<Instance *, set<Instance *>> _adj;
    vector<Instance *> _vertex;

public:
    void addEdge(Instance *u, Instance *v)
    {
        _adj[u].insert(v);
        _adj[v].insert(u);
    }
    void addVertex(Instance *v)
    {
        _vertex.push_back(v);
    }
    unordered_map<Instance *, set<Instance *>> adj()
    {
        return _adj;
    }
    vector<Instance *> vertex()
    {
        return _vertex;
    }
    // Finding all cliques in a graph is a complex problem.
    // Here's a simple method that finds and returns all cliques of size 3 (triangles) in the graph.
    std::vector<std::vector<Instance *>> findCliques()
    {
        std::vector<std::vector<Instance *>> cliques;

        for (auto &u : _adj)
        {
            for (auto &v : u.second)
            {
                for (auto &w : _adj[v])
                {
                    if (_adj[u.first].count(w) > 0)
                    {
                        cliques.push_back({u.first, v, w});
                    }
                }
            }
        }

        return cliques;
    }
};
;