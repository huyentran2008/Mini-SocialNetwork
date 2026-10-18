// BFS: khoang cach, duong ket noi (dung Queue) - khai bao
#ifndef BFS_ALGORITHM_H
#define BFS_ALGORITHM_H

#include <unordered_map>
#include <vector>
#include "core.h"

class BFSAlgorithm {
    const SocialNetwork& net;
public:
    explicit BFSAlgorithm(const SocialNetwork& n) : net(n) {}

    // Duyet theo tang tu src, ghi khoang cach va cha. maxDepth < 0: khong gioi han.
    void bfs(int src, std::unordered_map<int, int>& dist, std::unordered_map<int, int>& parent, int maxDepth = -1) const;

    int calculateDistance(int a, int b) const;

    std::vector<int> findShortestPath(int a, int b) const;

    // Nguoi dung cach nguon dung k buoc
    std::vector<int> getUsersAtDistance(int src, int k) const;

    bool isReachable(int a, int b) const { return calculateDistance(a, b) >= 0; }
};

#endif // BFS_ALGORITHM_H
