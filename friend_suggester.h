#ifndef FRIEND_SUGGESTER_H
#define FRIEND_SUGGESTER_H

#include "core.h"

// TiÃªu chÃ­ gá»£i Ã½ theo há»“ sÆ¡
enum SuggestKind { SUGGEST_HOMETOWN, SUGGEST_RESIDENCE, SUGGEST_SCHOOL, SUGGEST_WORKPLACE, SUGGEST_HOBBY };

vector<Suggestion> suggestByCommonGroup(const string& myId);
vector<Suggestion> suggestPopularUsers(const string& myId);
vector<Suggestion> computeSuggestions(const string& myId);
string suggestionLabel(const Suggestion& suggestion);

string normalizeProfileText(const string& text);
vector<Suggestion> suggestByProfile(const string& myId, SuggestKind kind);
string suggestionExtra(const Suggestion& suggestion);

void suggestionListScreen(
    const string& myId,
    const string& title,
    const function<vector<Suggestion>()>& source,
    const string& emptyMessage
);
void suggestionScreen(const string& myId);

#endif // FRIEND_SUGGESTER_H
