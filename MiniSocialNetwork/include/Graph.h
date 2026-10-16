#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <list>
#include <vector>
#include <iostream>

class Graph {
private:
    // Adjacency List: moi dinh la userId, danh sach ke la list cac ban be
    std::unordered_map<int, std::list<int>> adj;
    int edgeCount;

public:
    Graph();
    ~Graph();

    // Them dinh moi (nguoi dung)
    void addVertex(int vertexId);

    // Xoa dinh va moi canh lien quan
    void removeVertex(int vertexId);

    // Them canh vo huong (quan he ban be)
    void addEdge(int u, int v);

    // Xoa canh giua hai dinh
    void removeEdge(int u, int v);

    // Kiem tra dinh co ton tai khong
    bool hasVertex(int vertexId) const;

    // Kiem tra hai dinh co canh truc tiep khong
    bool isConnected(int u, int v) const;

    // Lay danh sach ke cua mot dinh
    std::list<int> getNeighbors(int vertexId) const;

    // Tra ve bac cua dinh (so ban)
    int getDegree(int vertexId) const;

    // Dem so dinh
    int getVertexCount() const;

    // Dem so canh
    int getEdgeCount() const;

    // Lay danh sach moi dinh
    std::vector<int> getAllVertices() const;

    // Xoa toan bo do thi
    void clear();

    friend std::ostream& operator<<(std::ostream& os, const Graph& graph);
};

#endif
