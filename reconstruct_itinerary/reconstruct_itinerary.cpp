class Solution {
public:
    unordered_map<string, map<string, int>> tt;
    vector<string> route;
    
    vector<string> findItinerary(vector<pair<string, string>> tickets) {
        // build src->dst map (counting the same src->dst tickets)
        for (auto ticket : tickets) {
            tt[ticket.first][ticket.second]++;
        }
        // start from JFK
        route.push_back("JFK");
        // visit all
        visit(tickets.size());
        return route;
    }
    bool visit(size_t count) {
        if (count == 0) {
            return true;
        }
        for (auto& it : tt[route.back()]) {
            if (it.second > 0) {
                route.push_back(it.first);
                it.second--;
                if (visit(count - 1)) {
                    return true;
                }
                it.second++;
                route.pop_back();
            }
        }
        return false;
    }
};
