#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stack>
#include <iomanip>

using namespace std;

// ==========================================
// ĐỒ THỊ DANH SÁCH KỀ (SIMPLE GRAPH)
// ==========================================
class SimpleGraph
{
private:
    unordered_map<int, unordered_set<int>> adjList;

public:
    void addVertex(int u)
    {
        if (adjList.find(u) == adjList.end())
        {
            adjList[u] = unordered_set<int>();
        }
    }

    void addEdge(int u, int v)
    {
        addVertex(u);
        addVertex(v);
        adjList[u].insert(v);
        adjList[v].insert(u);
    }

    const unordered_set<int> &getNeighbors(int u) const
    {
        static const unordered_set<int> emptySet;
        auto it = adjList.find(u);
        if (it != adjList.end())
            return it->second;
        return emptySet;
    }

    vector<int> getAllVertices() const
    {
        vector<int> vertices;
        for (const auto &pair : adjList)
        {
            vertices.push_back(pair.first);
        }
        return vertices;
    }

    int getVertexCount() const { return adjList.size(); }

    int getEdgeCount() const
    {
        int edges = 0;
        for (const auto &pair : adjList)
        {
            edges += pair.second.size();
        }
        return edges / 2; // Đồ thị vô hướng
    }

    int getDegree(int u) const
    {
        auto it = adjList.find(u);
        if (it != adjList.end())
            return it->second.size();
        return 0;
    }
};

// ==========================================
// 1. CLASS DSU: Quản lý các tập hợp rời nhau
// ==========================================
class DSU
{
private:
    unordered_map<int, int> parent;
    unordered_map<int, int> rank;

public:
    DSU() {}

    void makeSet(int u)
    {
        parent[u] = u;
        rank[u] = 0;
    }

    // Tìm đại diện tập hợp (Path Compression)
    int findSet(int u)
    {
        if (parent.find(u) == parent.end())
            return -1;
        if (u == parent[u])
            return u;
        return parent[u] = findSet(parent[u]);
    }

    // Hợp nhất 2 tập hợp (Union by Rank)
    bool unionSet(int u, int v)
    {
        int rootU = findSet(u);
        int rootV = findSet(v);

        if (rootU == -1 || rootV == -1)
            return false;
        if (rootU != rootV)
        {
            if (rank[rootU] < rank[rootV])
            {
                swap(rootU, rootV);
            }
            parent[rootV] = rootU;
            if (rank[rootU] == rank[rootV])
            {
                rank[rootU]++;
            }
            return true;
        }
        return false;
    }

    bool isSameSet(int u, int v)
    {
        int rootU = findSet(u);
        int rootV = findSet(v);
        return (rootU != -1 && rootV != -1 && rootU == rootV);
    }

    int getComponentCount(const vector<int> &vertices)
    {
        unordered_set<int> uniqueRoots;
        for (int v : vertices)
        {
            int root = findSet(v);
            if (root != -1)
                uniqueRoots.insert(root);
        }
        return uniqueRoots.size();
    }

    vector<int> getComponentMembers(int u, const vector<int> &vertices)
    {
        vector<int> members;
        int rootU = findSet(u);
        if (rootU == -1)
            return members;
        for (int v : vertices)
        {
            if (findSet(v) == rootU)
            {
                members.push_back(v);
            }
        }
        return members;
    }

    // Tái dựng DSU từ đồ thị
    void rebuildFromGraph(const SimpleGraph &graph)
    {
        parent.clear();
        rank.clear();
        vector<int> vertices = graph.getAllVertices();
        for (int v : vertices)
        {
            makeSet(v);
        }
        for (int u : vertices)
        {
            for (int v : graph.getNeighbors(u))
            {
                if (u < v)
                {
                    unionSet(u, v);
                }
            }
        }
    }
};

// ==========================================
// 2. CLASS DFSAlgorithm: Duyệt đồ thị theo chiều sâu
// ==========================================
class DFSAlgorithm
{
public:
    static void dfsRecursive(int u, const SimpleGraph &graph, unordered_set<int> &visited, vector<int> &component)
    {
        visited.insert(u);
        component.push_back(u);
        for (int v : graph.getNeighbors(u))
        {
            if (visited.find(v) == visited.end())
            {
                dfsRecursive(v, graph, visited, component);
            }
        }
    }

    static vector<int> collectComponent(int startUser, const SimpleGraph &graph)
    {
        vector<int> component;
        unordered_set<int> visited;
        stack<int> st;

        st.push(startUser);
        visited.insert(startUser);

        while (!st.empty())
        {
            int u = st.top();
            st.pop();
            component.push_back(u);

            for (int v : graph.getNeighbors(u))
            {
                if (visited.find(v) == visited.end())
                {
                    visited.insert(v);
                    st.push(v);
                }
            }
        }
        return component;
    }
};

// ==========================================
// 3. CLASS NetworkAnalyzer: Phân tích cấu trúc mạng lưới
// ==========================================
class NetworkAnalyzer
{
private:
    DSU &dsu;
    const SimpleGraph &graph;

public:
    NetworkAnalyzer(DSU &dsuRef, const SimpleGraph &graphRef) : dsu(dsuRef), graph(graphRef) {}

    bool areInSameComponent(int u, int v)
    {
        return dsu.isSameSet(u, v);
    }

    int countCommunities()
    {
        return dsu.getComponentCount(graph.getAllVertices());
    }

    vector<int> getLargestCommunity()
    {
        vector<int> vertices = graph.getAllVertices();
        unordered_map<int, vector<int>> components;

        for (int v : vertices)
        {
            int root = dsu.findSet(v);
            if (root != -1)
            {
                components[root].push_back(v);
            }
        }

        vector<int> largest;
        for (const auto &pair : components)
        {
            if (pair.second.size() > largest.size())
            {
                largest = pair.second;
            }
        }
        return largest;
    }

    vector<int> getCommunityOfUser(int u)
    {
        return dsu.getComponentMembers(u, graph.getAllVertices());
    }

    int findMostConnectedUser()
    {
        vector<int> vertices = graph.getAllVertices();
        int maxDegree = -1;
        int mostConnectedUser = -1;

        for (int v : vertices)
        {
            int degree = graph.getDegree(v);
            if (degree > maxDegree)
            {
                maxDegree = degree;
                mostConnectedUser = v;
            }
        }
        return mostConnectedUser;
    }

    double getAverageDegree()
    {
        int vertexCount = graph.getVertexCount();
        if (vertexCount == 0)
            return 0.0;
        return (2.0 * graph.getEdgeCount()) / vertexCount;
    }

    vector<int> findIsolatedUsers()
    {
        vector<int> isolated;
        for (int v : graph.getAllVertices())
        {
            if (graph.getDegree(v) == 0)
            {
                isolated.push_back(v);
            }
        }
        return isolated;
    }
};

// ==========================================
// 4. CLASS TarjanAlgorithm: Phân tích đặc điểm liên thông
// ==========================================
class TarjanAlgorithm
{
private:
    int timer;
    unordered_map<int, int> disc, low;
    unordered_set<int> visited;
    vector<int> articulationPoints;
    vector<pair<int, int>> bridges;

    void tarjanDFS(int u, int parent, const SimpleGraph &graph)
    {
        visited.insert(u);
        disc[u] = low[u] = ++timer;
        int children = 0;
        bool isArticulation = false;

        for (int v : graph.getNeighbors(u))
        {
            if (v == parent)
                continue;

            if (visited.find(v) != visited.end())
            {
                low[u] = min(low[u], disc[v]);
            }
            else
            {
                children++;
                tarjanDFS(v, u, graph);
                low[u] = min(low[u], low[v]);

                if (parent != -1 && low[v] >= disc[u])
                {
                    isArticulation = true;
                }
                if (low[v] > disc[u])
                {
                    bridges.push_back({u, v});
                }
            }
        }

        if (parent == -1 && children > 1)
        {
            isArticulation = true;
        }

        if (isArticulation)
        {
            articulationPoints.push_back(u);
        }
    }

public:
    void run(const SimpleGraph &graph)
    {
        timer = 0;
        disc.clear();
        low.clear();
        visited.clear();
        articulationPoints.clear();
        bridges.clear();

        for (int v : graph.getAllVertices())
        {
            if (visited.find(v) == visited.end())
            {
                tarjanDFS(v, -1, graph);
            }
        }
    }

    vector<int> getArticulationPoints() const { return articulationPoints; }
    vector<pair<int, int>> getBridges() const { return bridges; }
};

// ==========================================
// MAIN DEMO MODULE 3.4
// ==========================================
int main()
{
    SimpleGraph graph;

    // Thiết lập dữ liệu đồ thị mạng xã hội
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);
    graph.addEdge(3, 4);
    graph.addEdge(1, 3);
    graph.addEdge(5, 6);
    graph.addVertex(7); // Người dùng cô lập

    // Khởi tạo và nạp dữ liệu DSU
    DSU dsu;
    dsu.rebuildFromGraph(graph);

    // Khởi tạo các module phân tích
    NetworkAnalyzer analyzer(dsu, graph);
    TarjanAlgorithm tarjan;
    tarjan.run(graph);

    // Hiển thị kết quả thống kê
    cout << "=== 3.4. MODULE LIEN THONG VA THONG KE ===\n\n";
    cout << "1. Tong so nguoi dung: " << graph.getVertexCount() << endl;
    cout << "2. Tong so quan he ket ban: " << graph.getEdgeCount() << endl;
    cout << "3. So luong cong dong (Thanh phan lien thong): " << analyzer.countCommunities() << endl;
    cout << "4. So ban trung binh: " << fixed << setprecision(2) << analyzer.getAverageDegree() << endl;

    int topUser = analyzer.findMostConnectedUser();
    cout << "5. Nguoi co nhieu ban nhat: User " << topUser << " (" << graph.getDegree(topUser) << " ban)\n";

    cout << "\n--- KIEM TRA LIEN THONG ---\n";
    cout << "User 1 va User 4: " << (analyzer.areInSameComponent(1, 4) ? "Cung nhom" : "Khac nhom") << endl;
    cout << "User 1 va User 5: " << (analyzer.areInSameComponent(1, 5) ? "Cung nhom" : "Khac nhom") << endl;

    cout << "\n--- PHAN TICH THUAT TOAN TARJAN ---\n";
    cout << "Nguoi cau noi (Articulation Points): ";
    for (int p : tarjan.getArticulationPoints())
        cout << p << " ";

    cout << "\nMoi quan he then chot (Bridges): ";
    for (auto b : tarjan.getBridges())
        cout << "(" << b.first << " - " << b.second << ") ";
    cout << "\n";

    return 0;
}