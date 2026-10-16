#include "Graph.h"
#include <algorithm>

Graph::Graph() : edgeCount(0) {}

Graph::~Graph() {
    clear();
}

void Graph::addVertex(int vertex) {
    if (!hasVertex(vertex)) {
        adjList[vertex] = std::list<int>();
    }
}

void Graph::removeVertex(int vertex) {
    if (!hasVertex(vertex)) return;

    // Remove all edges connected to this vertex
    for (auto& pair : adjList) {
        pair.second.remove(vertex);
    }
    // Count edges removed
    edgeCount -= static_cast<int>(adjList[vertex].size());
    adjList.erase(vertex);
}

void Graph::addEdge(int u, int v) {
    if (u == v) return;
    addVertex(u);
    addVertex(v);

    // Check if edge already exists
    auto& listU = adjList[u];
    if (std::find(listU.begin(), listU.end(), v) == listU.end()) {
        listU.push_back(v);
        adjList[v].push_back(u);
        ++edgeCount;
    }
}

void Graph::removeEdge(int u, int v) {
    if (!hasVertex(u) || !hasVertex(v)) return;

    auto& listU = adjList[u];
    auto it = std::find(listU.begin(), listU.end(), v);
    if (it != listU.end()) {
        listU.erase(it);
        adjList[v].remove(u);
        --edgeCount;
    }
}

bool Graph::hasVertex(int vertex) const {
    return adjList.find(vertex) != adjList.end();
}

bool Graph::isConnected(int u, int v) const {
    if (!hasVertex(u) || !hasVertex(v)) return false;
    if (u == v) return true;

    std::set<int> visited;
    std::queue<int> q;
    q.push(u);
    visited.insert(u);

    while (!q.empty()) {
        int current = q.front();
        q.pop();
        if (current == v) return true;

        auto it = adjList.find(current);
        if (it != adjList.end()) {
            for (int neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }
    return false;
}

std::vector<int> Graph::getNeighbors(int vertex) const {
    std::vector<int> result;
    auto it = adjList.find(vertex);
    if (it != adjList.end()) {
        for (int n : it->second) {
            result.push_back(n);
        }
    }
    return result;
}

int Graph::getDegree(int vertex) const {
    auto it = adjList.find(vertex);
    if (it != adjList.end()) {
        return static_cast<int>(it->second.size());
    }
    return 0;
}

int Graph::getVertexCount() const {
    return static_cast<int>(adjList.size());
}

int Graph::getEdgeCount() const {
    return edgeCount;
}

std::vector<int> Graph::getAllVertices() const {
    std::vector<int> result;
    for (const auto& pair : adjList) {
        result.push_back(pair.first);
    }
    return result;
}

void Graph::clear() {
    adjList.clear();
    edgeCount = 0;
}

std::vector<int> Graph::findPathBFS(int start, int end) const {
    std::vector<int> path;
    if (!hasVertex(start) || !hasVertex(end)) return path;
    if (start == end) {
        path.push_back(start);
        return path;
    }

    std::map<int, int> parent;
    std::queue<int> q;
    std::set<int> visited;
    q.push(start);
    visited.insert(start);
    parent[start] = -1;
    bool found = false;

    while (!q.empty() && !found) {
        int current = q.front();
        q.pop();
        auto it = adjList.find(current);
        if (it != adjList.end()) {
            for (int neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    q.push(neighbor);
                    if (neighbor == end) {
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    if (found) {
        int cur = end;
        while (cur != -1) {
            path.push_back(cur);
            cur = parent[cur];
        }
        std::reverse(path.begin(), path.end());
    }
    return path;
}

std::vector<int> Graph::findPathDFS(int start, int end) const {
    std::vector<int> path;
    if (!hasVertex(start) || !hasVertex(end)) return path;

    std::map<int, int> parent;
    std::stack<int> s;
    std::set<int> visited;
    s.push(start);
    visited.insert(start);
    parent[start] = -1;
    bool found = false;

    while (!s.empty() && !found) {
        int current = s.top();
        s.pop();
        if (current == end) {
            found = true;
            break;
        }
        auto it = adjList.find(current);
        if (it != adjList.end()) {
            for (int neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    s.push(neighbor);
                }
            }
        }
    }

    if (found) {
        int cur = end;
        while (cur != -1) {
            path.push_back(cur);
            cur = parent[cur];
        }
        std::reverse(path.begin(), path.end());
    }
    return path;
}
