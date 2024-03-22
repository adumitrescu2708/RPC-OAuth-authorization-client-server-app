#include "end_user.h"
/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | END USER SOURCE CODE

*/

queue<string> aprovals;
char noPermisions[] = "*,-";

int counter = 0;

/* 
    Method used for parsing the given action.
    Checks the format and returns False in case of "*,-" string found.
    Otherwise, split the string into pairs of <resource, permsissions>
    and create an unordered_map that is sent to the database
*/
bool addPermisions(char *clientId, string aproval) {
    if(aproval.compare(noPermisions) == 0) {
        return false;
    } else {
        char *s = &aproval[0];
        char token[] = ",";
        char *p = strtok(s, token);
        unordered_map<string, string> permisions;
        while(p != NULL) {
            permisions[string(p)] = strtok(NULL, token);
            p = strtok(NULL, token);
        }
        setUserPermissions(clientId, permisions);
        return true;
    }
}

/*
    Considering the FIFO logic, first user to come receives the first available
    action.
*/
bool validateEndUser(char *clientId, char *authToken) {
    string aproval = aprovals.front();
    aprovals.pop();

    return addPermisions(clientId, aproval);
}

/* 
    Method used for parsing the input file 
    Creates a queue of actions in string format.
*/
bool parseAprovals(char *filename) {
    ifstream inputFile;
    inputFile.open(filename);
    if(!inputFile.is_open()) {
        cerr << "File not found " << filename << "!"  << endl;
        return false;
    }

    string line;

    while(inputFile >> line) {
        aprovals.push(line);
    }

    inputFile.close();
    
    return true;
}