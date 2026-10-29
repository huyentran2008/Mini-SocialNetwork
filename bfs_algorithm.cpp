#include "bfs_algorithm.h"
#include "sort_service.h"

vector<Suggestion> suggestByFriendGraph(const string& myId) {
    map<string, vector<string>> adjacency;

    auto neighbors = [&](const string& id) -> const vector<string>& {
        auto it = adjacency.find(id);

        if (it == adjacency.end()) {
            it = adjacency.emplace(id, getFriendIds(id)).first;
        }

        return it->second;
    };

    map<string, int> distance;
    vector<string> order;

    distance[myId] = 0;
    order.push_back(myId);

    for (size_t head = 0; head < order.size(); head++) {
        string current = order[head];
        int d = distance[current];

        if (d >= SUGGEST_MAX_DEPTH) {
            continue;
        }

        for (const string& next : neighbors(current)) {
            if (distance.count(next)) {
                continue;
            }

            distance[next] = d + 1;
            order.push_back(next);
        }
    }

    vector<Suggestion> list;

    for (const string& id : order) {
        int d = distance[id];
        if (d < 2) {
            continue;
        }

        UserProfile profile;

        if (!canSuggestTo(myId, id, profile)) {
            continue;
        }

        Suggestion suggestion;
        suggestion.id = id;
        suggestion.name = profile.fullName.empty() ? profile.username : profile.fullName;
        suggestion.distance = d;
        suggestion.mutual = countMutualFriends(myId, id);
        suggestion.friendCount = (int)neighbors(id).size();
        list.push_back(suggestion);
    }

    sortSuggestionsByGraph(list);

    if ((int)list.size() > SUGGEST_MAX_RESULT) {
        list.resize(SUGGEST_MAX_RESULT);
    }

    return list;
}
