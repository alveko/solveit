#include <iostream>
#include <climits>
#include <iterator>
#include <vector>
#include <sstream>
#include "simple_database.hpp"

using namespace std;
using namespace SimpleDatabase;

int main() {
    const int NullInt = INT_MIN;
    using SimpleDatabaseStrInt = SimpleDatabaseHash<string, int, NullInt>;
    SimpleDatabaseStrInt db;

    string line;
    while (getline(cin, line)) {
        istringstream iss(line);
        istream_iterator<string> iit(iss), eos;
        vector<string> tokens(iit, eos);
        //cout << "# " << line << endl;

        // for simplicity, using if-else-if here; normally it would be a map
        if (tokens.empty() || tokens[0] == "END") {
            break;
        } else if (tokens[0] == "SET" && tokens.size() == 3) {
            db.set(tokens[1], stoi(tokens[2]));
        } else if (tokens[0] == "UNSET" && tokens.size() == 2) {
            db.unset(tokens[1]);
        } else if (tokens[0] == "GET" && tokens.size() == 2) {
            int value = db.get(tokens[1]);
            if (value != NullInt) {
                cout << value << endl;
            } else {
                cout << "NULL" << endl;
            }
        } else if (tokens[0] == "NUMEQUALTO" && tokens.size() == 2) {
            cout << db.countEqualTo(stoi(tokens[1])) << endl;
        } else if (tokens[0] == "BEGIN") {
            db.begin();
        } else if (tokens[0] == "ROLLBACK") {
            if (!db.rollback()) {
                cout << "NO TRANSACTION" << endl;
            }
        } else if (tokens[0] == "COMMIT") {
            if (!db.commit()) {
                cout << "NO TRANSACTION" << endl;
            }
        } else {
            cout << "Unknown or malformed command:" << endl;
            cout << line << endl;
            break;
        }
    }
    return 0;
}
