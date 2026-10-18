// BFS: khoang cach, duong ket noi (dung Queue) - cai dat
#include "bfs_algorithm.h"
#include <algorithm>
#include <queue>

using namespace std;

void BFSAlgorithm::bfs(int src, unordered_map<int, int>& dist, unordered_map<int, int>& parent, int maxDepth) const {
    dist.clear(); parent.clear();
    if (!net.exists(src)) return;
    queue<int> q;
    dist[src] = 0;
    parent[src] = -1;
    q.push(src);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (maxDepth >= 0 && dist[u] >= maxDepth) continue;
        const User* usr = net.getUser(u);
        for (set<int>::const_iterator it = usr->friends.begin(); it != usr->friends.end(); ++it) {
            int v = *it;
            if (dist.find(v) == dist.end()) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }
}

int BFSAlgorithm::calculateDistance(int a, int b) const {
    if (!net.exists(a) || !net.exists(b)) return -1;
    unordered_map<int, int> dist, parent;
    bfs(a, dist, parent);
    unordered_map<int, int>::const_iterator it = dist.find(b);
    return it == dist.end() ? -1 : it->second;
}

vector<int> BFSAlgorithm::findShortestPath(int a, int b) const {
    vector<int> path;
    if (!net.exists(a) || !net.exists(b)) return path;
    unordered_map<int, int> dist, parent;
    bfs(a, dist, parent);
    if (dist.find(b) == dist.end()) return path;
    for (int cur = b; cur != -1; cur = parent[cur]) path.push_back(cur);
    reverse(path.begin(), path.end());
    return path;
}

vector<int> BFSAlgorithm::getUsersAtDistance(int src, int k) const {
    vector<int> res;
    unordered_map<int, int> dist, parent;
    bfs(src, dist, parent, k);
    for (unordered_map<int, int>::const_iterator it = dist.begin(); it != dist.end(); ++it)
        if (it->second == k) res.push_back(it->first);
    sort(res.begin(), res.end());
    return res;
}
