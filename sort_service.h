// SORT: merge sort tu cai + sap xep nguoi dung - khai bao
#ifndef SORT_SERVICE_H
#define SORT_SERVICE_H

#include <vector>
#include "core.h"

// Merge sort tu cai (template nen nam trong .h)
template <typename T, typename Less>
void mergeSortImpl(std::vector<T>& a, std::vector<T>& tmp, int l, int r, Less less) {
    if (r - l <= 1) return;
    int m = (l + r) / 2;
    mergeSortImpl(a, tmp, l, m, less);
    mergeSortImpl(a, tmp, m, r, less);
    int i = l, j = m, k = l;
    while (i < m && j < r) {
        if (less(a[j], a[i])) tmp[k++] = a[j++];
        else tmp[k++] = a[i++];
    }
    while (i < m) tmp[k++] = a[i++];
    while (j < r) tmp[k++] = a[j++];
    for (int x = l; x < r; ++x) a[x] = tmp[x];
}

template <typename T, typename Less>
void mergeSort(std::vector<T>& a, Less less) {
    std::vector<T> tmp(a.size());
    mergeSortImpl(a, tmp, 0, (int)a.size(), less);
}

class SortService {
    const SocialNetwork& net;
public:
    explicit SortService(const SocialNetwork& n) : net(n) {}

    // --- sap xep (tu cai) ---
    // Theo ten kieu Viet Nam: so ten (tu cuoi) truoc, roi den ca ho ten, cuoi cung theo ID
    std::vector<int> sortUsersByName(std::vector<int> ids) const;

    // Giam dan theo so ban be
    std::vector<int> sortUsersByFriendCount(std::vector<int> ids) const;
};

#endif // SORT_SERVICE_H
