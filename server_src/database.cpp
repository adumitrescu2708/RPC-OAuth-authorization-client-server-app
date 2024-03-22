/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | DATABASE SOURCE CODE

*/
#include "database.h"

using namespace std;

/* RESOURCES */
vector<string>          resources;
int                     resourcesCount;

/* CLIENTS & TOKENS */
int                              valability;
int                              usersCount;
vector<string>                   usersIds;
unordered_map<string, string>    users;
unordered_map<string, int>       usersValability;

/* PERMISSIONS */
unordered_map<string, unordered_map<string, string>> usersPermisions;

/*
    Function that iterates over the entries in the users map
    in order to retrieve the user id corresponding to the given access
    token.
*/
string getId(char *token) {
    string str_token = string(token);

    for(auto pair : users) {
        if(pair.second.compare(str_token) == 0)
            return pair.first;
    }
    
    /* In case of a not found access token, return ERROR string */
    return string(ERROR);
}

/*
    If a client has selected automatic refresh access token, once the request
    from user is received, it needs to
        1. Update valability (number of permitted operations)
        2. Replace the old access token with the newly generated one
*/
void refreshDatabase(char *clientId, char *newAccessToken) {
    /* (1) */
    usersValability[clientId]   = valability;
    /* (2) */
    users[clientId]             = newAccessToken;
}

/*
    @param1  - client id
    @param2  - targeted resource
    @param3  - desired action (enum list is found in application.x file)
    @returns - True (if the user is allowed to execute the desired action) or False (if not)

    Function that validated the RIMDX operations depending on each user's permisions
        1.  Retrieve the client's permissions from the usersPermisions mapping the user id
            to a map of resources and allowed operations on the corresponding resource.
        2.  Check if the desired action is UKNOWN, in which case the user is not allowed
            to do anything and no action can take place. 
        3.  Check if the RIMDX operation can take place.
*/
bool checkPermissions(char *clientId, char *resource, actionCodes action) {
    /* (1) */
    string permission = usersPermisions[clientId][resource];

    /* (2) */
    if(action == UNKNOWN)
        return false;
    
    /* (3) */
    if(action == READ) {
        if(permission.find('R') != string::npos)
            return true;
        return false;
    }
    if(action == MODIFY) {
        if(permission.find('M') != string::npos)
            return true;
        return false;
    }
    if(action == DELETE) {
        if(permission.find('D') != string::npos)
            return true;
        return false;
    }
    if(action == INSERT) {
        if(permission.find('I') != string::npos)
            return true;
        return false;
    }
    if(action == EXECUTE) {
        if(permission.find('X') != string::npos)
            return true;
        return false;
    }
    return false;
}

/*
    Return remained availability for the current client's access token.
    This function is separated from the server side, implemented in the
    database logic.
*/
int getAvailability(char *clientId) {
    return usersValability[clientId];
}

/*
    This function is called by the external user each time an authorization
    token is being signed. Stores the mapping from resource to permissions
    in the usersPermisions map at the client id index.
*/
void setUserPermissions(char * clientId, unordered_map<string, string> perm) {
    usersPermisions[clientId] = perm;
}

/*
    Each time an user executed a RIMDX operation, its access token's
    availability decrements.
*/
void decrementAvailability(char *clientId) {
    usersValability[clientId] -= 1;
}

/*
    Returns the standars user availability of the access token.
    This function is separated from the server side, implemented in the
    database logic.
*/
int getValability() {
    return valability;
}

/*
    Check if the given resource is in the list of resources.
*/
bool checkResource(char *resource) {
    if(find(resources.begin(), resources.end(), resource) != resources.end())
        return true;
    return false;
}

/*
    Check if the user has more availability for the current access token.
*/
bool checkAvailabilityToken(char *clientId) {
    return usersValability[clientId] > 0;
}

/*
    Check if the given client id is in the map containing all users id.
*/
bool checkAccessToken(char *clientId, char *clientAccessToken) {
    if(users.find(clientId) == users.end())
        return false;

    if(strcmp(users[clientId].c_str(), clientAccessToken) == 0)
        return true;

    return false;
}

/*
    Function that adds an access token of a user in the database.
    It maps the given client id to the access token and then sets
    its valability to the standard user valability of the access token.
*/
void addAccessToken(char *clientId, char *accessToken) {
    users[(clientId)]             = accessToken;
    usersValability[(clientId)]   = valability;
}

/*
    Stores the authorization token in the database.
*/
void addAuthToken(char *clientId, char *token) {
    users[(clientId)] = token;
}

/*
    Check if the user id is in the list of clients ids.
*/
bool checkUserId(char *id) {
    if(!id)
        return false;

    for(int i = 0; i < usersCount; i++) {
        if(strcmp(usersIds.at(i).c_str(), id) == 0)
            return true;
    }
    return false;
}

/*
    Parses input file with known users ids from server side.
*/
bool parseInputIds(char *filename) {
    ifstream inputFile;
    inputFile.open(filename);

    /*
        Throws error if the file is not found.
    */
    if(!inputFile.is_open()) {
        cerr << "File not found " << filename << "!"  << endl;
        return false;
    }

    string ID;

    inputFile >> usersCount;
    for(int i = 0; i < usersCount; i++) {
        inputFile >> ID;
        usersIds.push_back(ID);
    }

    inputFile.close();
    return true;
}

/*
    Parses the resources input file, storing the resources in a vector
*/
bool parseResources(char *filename) {
    ifstream inputFile;
    inputFile.open(filename);

    /* Throws error if the resources file is not found */
    if(!inputFile.is_open()) {
        cerr << "File not found " << filename << "!" << endl;
        return false;
    }

    /* Store resources in the resources vector */
    string resource;
    inputFile >> resourcesCount;
    for(int i = 0; i < resourcesCount; i++) {
        inputFile >> resource;
        resources.push_back(resource);
    }

    inputFile.close();
    return true;
}

/*
    Parses the given standard valability for any user access token.
*/
void setValabilityTokens(char *token) {
    valability = atoi(token);
}