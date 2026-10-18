// GOI Y: ket ban theo ban chung va thuoc tinh - khai bao
#ifndef FRIEND_SUGGESTER_H
#define FRIEND_SUGGESTER_H

#include <string>
#include <vector>
#include "core.h"
#include "bfs_algorithm.h"

struct Suggestion {
    int userId;
    int mutual;
    int score;
    std::vector<std::string> reasons;   // moi ly do mot dong
    Suggestion() : userId(0), mutual(0), score(0) {}
};

class FriendSuggester {
    const SocialNetwork& net;
    const BFSAlgorithm& bfsAlgo;

    static bool sameValue(const std::string& a, const std::string& b);
    std::vector<std::string> commonHobbies(const User& a, const User& b) const;
    // Gia tri thuoc tinh cua nguoi u co chua mot trong cac tu khoa
    static std::vector<std::string> matchedValues(const User& u, Attr a, const std::vector<std::string>& keys);
    // Chuyen danh sach id ung vien thanh Suggestion (loai ban than + ban hien tai), co diem va ly do.
    // attr >= 0: ung vien duoc chon vi thuoc tinh do khop tu khoa -> cong 5 diem va ghi ro gia tri khop.
    std::vector<Suggestion> buildSuggestions(int uid, const std::vector<int>& ids, int attr = -1,
                                        const std::vector<std::string>& keys = std::vector<std::string>()) const;
public:
    FriendSuggester(const SocialNetwork& n, const BFSAlgorithm& b) : net(n), bfsAlgo(b) {}

    int countMutualFriends(int a, int b) const { return (int)net.mutualFriends(a, b).size(); }

    // Giam dan theo diem; hoa thi nhieu ban chung hon; hoa nua thi id nho truoc
    void rankSuggestions(std::vector<Suggestion>& v) const;

    // Ban cua ban (cach dung 2 buoc theo BFS), xep theo so ban chung
    std::vector<Suggestion> suggestFriends(int uid) const;

    std::vector<int> collectCandidatesByMutual(int uid) const { return bfsAlgo.getUsersAtDistance(uid, 2); }

    // Ung vien co thuoc tinh trung voi minh (lay tu AttributeIndex)
    std::vector<int> collectCandidatesByAttribute(int uid) const;

    // Gop hai nguon: loai trung, loai nguoi da la ban va chinh minh
    std::vector<int> mergeCandidates(const std::vector<int>& byMutual, const std::vector<int>& byAttr, int uid) const;

    // Diem = 10*ban chung + 4*cung truong + 4*cung noi lam viec + 3*cung noi o + 3*cung que + 2*moi so thich chung
    int calculateScore(int uid, int cand) const;

    // Cac ly do goi y, moi ly do mot dong; rong neu khong co gi chung
    std::vector<std::string> getReasons(int uid, int cand) const;

    // Goi y tong hop (ban chung + thuoc tinh): tat ca ung vien, xep theo diem giam dan
    std::vector<Suggestion> suggestAll(int uid) const;

    // Goi y theo MOT thuoc tinh (noi o / que quan / truong / noi lam viec / so thich).
    // keyword khong rong: lay nguoi co thuoc tinh CHUA tu khoa ("hat" khop "ca hat").
    // keyword rong: dung chinh cac gia tri thuoc tinh cua nguoi can goi y lam tu khoa.
    std::vector<Suggestion> suggestByAttribute(int uid, Attr a, const std::string& keyword) const;

    // Goi y ban trong cung nhom
    std::vector<Suggestion> suggestInGroup(int uid, int gid) const;
};

#endif // FRIEND_SUGGESTER_H
