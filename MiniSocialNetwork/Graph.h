#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <set>

class Graph {
private:
    std::map<int, std::list<int>> adjList; // userId -> list of friend userIds
    int edgeCount;

public:
    Graph();
    ~Graph();

    // Required methods
    void addVertex(int vertex);
    void removeVertex(int vertex);
    void addEdge(int u, int v);
    void removeEdge(int u, int v);
    bool hasVertex(int vertex) const;
    bool isConnected(int u, int v) const; // BFS based
    std::vector<int> getNeighbors(int vertex) const;
    int getDegree(int vertex) const;
    int getVertexCount() const;
    int getEdgeCount() const;
    std::vector<int> getAllVertices() const;
    void clear();

    // Additional for path finding
    std::vector<int> findPathBFS(int start, int end) const;
    std::vector<int> findPathDFS(int start, int end) const;
};

#endif
