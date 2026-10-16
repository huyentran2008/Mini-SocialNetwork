#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stack>
#include <iomanip>

using namespace std;

// ==========================================
// 1. CẤU TRÚC ĐỒ THỊ GIẢ LẬP (ADJACENCY LIST GRAPH)
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
// 2. DISJOINT SET UNION (DSU) MODULE
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

    // Tim đại diện của tập hợp (Nén đường đi - Path Compression)
    int findSet(int u)
    {
        if (parent.find(u) == parent.end())
            return -1;
        if (u == parent[u])
            return u;
        return parent[u] = findSet(parent[u]);
    }

    // Hợp nhất hai tập hợp (Hợp theo hạng - Union by Rank)
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

    int getComponentSize(int u, const vector<int> &vertices)
    {
        int rootU = findSet(u);
        if (rootU == -1)
            return 0;
        int count = 0;
        for (int v : vertices)
        {
            if (findSet(v) == rootU)
                count++;
        }
        return count;
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

    // Dựng lại DSU từ đồ thị
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
                { // Tránh hợp nhất 2 lần cho cạnh vô hướng
                    unionSet(u, v);
                }
            }
        }
    }
};

// ==========================================
// 3. THUẬT TOÁN DFS (DFSALGORITHM)
// ==========================================
class DFSAlgorithm
{
public:
    // DFS đệ quy thu thập thành phần liên thông
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

    // DFS khử đệ quy dùng Stack tự cài/STL
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
// 4. NETWORK ANALYZER (PHÂN TÍCH & THỐNG KÊ MẠNG)
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
// 5. TARJAN ALGORITHM (TÌM KHỚP VÀ CẦU - TÙY CHỌN NÂNG CAO)
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

                // Điều kiện điểm khớp (Người cầu nối)
                if (parent != -1 && low[v] >= disc[u])
                {
                    isArticulation = true;
                }
                // Điều kiện cạnh cầu (Mối quan hệ then chốt)
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
// DEMO CHƯƠNG TRÌNH THỰC THI
// ==========================================
int main()
{
    SimpleGraph g;

    // Tạo mạng xã hội mẫu:
    // Nhóm 1: {1, 2, 3, 4} (Thành phần liên thông 1)
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(1, 3);

    // Nhóm 2: {5, 6} (Thành phần liên thông 2)
    g.addEdge(5, 6);

    // Nhóm 3: {7} (Người cô lập)
    g.addVertex(7);

    // Khởi tạo DSU và nạp dữ liệu
    DSU dsu;
    dsu.rebuildFromGraph(g);

    // Phân tích mạng xã hội
    NetworkAnalyzer analyzer(dsu, g);

    cout << "================ BANG THONG KE MANG XA HOI ================\n";
    cout << "1. Tong so nguoi dung: " << g.getVertexCount() << endl;
    cout << "2. Tong so quan he (canh): " << g.getEdgeCount() << endl;
    cout << "3. So nhom/cong dong (Lien thong): " << analyzer.countCommunities() << endl;
    cout << "4. So ban trung binh / nguoi: " << fixed << setprecision(2) << analyzer.getAverageDegree() << endl;

    int topUser = analyzer.findMostConnectedUser();
    cout << "5. Nguoi co nhieu ban nhat (User ID): " << topUser << " (voi " << g.getDegree(topUser) << " ban)" << endl;

    cout << "6. Kiem tra lien thong giua ID 1 va ID 4: "
         << (analyzer.areInSameComponent(1, 4) ? "Cung nhom" : "Khac nhom") << endl;
    cout << "7. Kiem tra lien thong giua ID 1 va ID 5: "
         << (analyzer.areInSameComponent(1, 5) ? "Cung nhom" : "Khac nhom") << endl;

    vector<int> largestGroup = analyzer.getLargestCommunity();
    cout << "8. Danh sach thanh vien nhom lon nhat: ";
    for (int id : largestGroup)
        cout << id << " ";
    cout << endl;

    vector<int> isolated = analyzer.findIsolatedUsers();
    cout << "9. Danh sach nguoi co lap (Chua co ban): ";
    for (int id : isolated)
        cout << id << " ";
    cout << endl;

    // Chạy thuật toán Tarjan
    TarjanAlgorithm tarjan;
    tarjan.run(g);

    cout << "\n================ THUAT TOAN TARJAN (NANG CAO) ================\n";
    cout << "10. Nguoi cau noi (Articulation Points): ";
    for (int p : tarjan.getArticulationPoints())
        cout << p << " ";
    cout << "\n11. Quan he then chot (Bridges): ";
    for (auto b : tarjan.getBridges())
        cout << "(" << b.first << " - " << b.second << ") ";
    cout << "\n---------------------------------------------------------------\n";

    return 0;
}