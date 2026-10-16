#include "../include/Graph.h"
#include <algorithm>

Graph::Graph() : edgeCount(0) {}

Graph::~Graph() {
    clear();
}

void Graph::addVertex(int vertexId) {
    if (adj.find(vertexId) == adj.end()) {
        adj[vertexId] = std::list<int>();
    }
}

void Graph::removeVertex(int vertexId) {
    if (adj.find(vertexId) == adj.end()) return;

    // Xoa moi canh lien quan
    for (int neighbor : adj[vertexId]) {
        adj[neighbor].remove(vertexId);
        --edgeCount;
    }
    adj.erase(vertexId);
}

void Graph::addEdge(int u, int v) {
    if (u == v) return; // khong tu ket ban

    addVertex(u);
    addVertex(v);

    // Kiem tra da co canh chua
    for (int neighbor : adj[u]) {
        if (neighbor == v) return;
    }

    adj[u].push_back(v);
    adj[v].push_back(u);
    ++edgeCount;
}

void Graph::removeEdge(int u, int v) {
    if (adj.find(u) == adj.end() || adj.find(v) == adj.end()) return;

    size_t beforeU = adj[u].size();
    adj[u].remove(v);
    adj[v].remove(u);

    if (adj[u].size() < beforeU) {
        --edgeCount;
    }
}

bool Graph::hasVertex(int vertexId) const {
    return adj.find(vertexId) != adj.end();
}

bool Graph::isConnected(int u, int v) const {
    if (adj.find(u) == adj.end()) return false;
    for (int neighbor : adj.at(u)) {
        if (neighbor == v) return true;
    }
    return false;
}

std::list<int> Graph::getNeighbors(int vertexId) const {
    if (adj.find(vertexId) == adj.end()) {
        return std::list<int>();
    }
    return adj.at(vertexId);
}

int Graph::getDegree(int vertexId) const {
    if (adj.find(vertexId) == adj.end()) return 0;
    return static_cast<int>(adj.at(vertexId).size());
}

int Graph::getVertexCount() const {
    return static_cast<int>(adj.size());
}

int Graph::getEdgeCount() const {
    return edgeCount;
}

std::vector<int> Graph::getAllVertices() const {
    std::vector<int> vertices;
    for (const auto& pair : adj) {
        vertices.push_back(pair.first);
    }
    return vertices;
}

void Graph::clear() {
    adj.clear();
    edgeCount = 0;
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    os << "Graph: " << graph.getVertexCount() << " vertices, "
       << graph.getEdgeCount() << " edges\n";
    for (const auto& pair : graph.adj) {
        os << "  " << pair.first << " -> ";
        for (int neighbor : pair.second) {
            os << neighbor << " ";
        }
        os << "\n";
    }
    return os;
}
