#ifndef SORT_SERVICE_H
#define SORT_SERVICE_H

#include "core.h"
void sortSuggestionsByGraph(vector<Suggestion>& list);
void sortSuggestionsByCommonGroup(vector<Suggestion>& list);
void sortSuggestionsByPopularity(vector<Suggestion>& list);
void sortSuggestionsByProfile(vector<Suggestion>& list);
void sortUsersByNameMatch(vector<UserProfile>& matched, const string& key);

#endif // SORT_SERVICE_H
