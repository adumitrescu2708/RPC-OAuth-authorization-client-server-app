/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | CLIENT HELPER'S SOURCE CODE

*/

#include"client_helper.h"

using namespace std;

queue<struct queue_entry_t>     actions;                /* queue of input commands */
unordered_map<string, string>   usersAccessTokens;      /* unordered_map used for mapping client's id to access token */
unordered_map<string, string>   usersRefreshTokens;     /* unordered_map used for mapping client's id to refresh token */
unordered_map<string, int>      usersValability;        /* unordered_map used for mapping client's id to valability */

/*
    REQUEST action
    
    The request action is split in 3 stages:
    
    (1.) Request authorization (to server):

        Send authRequest_t package storing the client's id and store the result. In the result package,
        there is an error code showing wether the operation has been completed successfully or not.
        In case of error print the log.

    (2.) Approve request token (to end user):

        Send the previously received authorization token to the end user and receive
        a new token, that could be signed by the end user or not.

    (3.) Request access token (to server):

        Sends client's id, client's authorization token, command argument (could be 0 or 1).
        If 0 argument is given, the server will not send back a refresh token, so it will not be
        logged. If the 1 argument is given, store the user's valability in order to keep track
        of when the refresh of the access token should take place and store the refresh token.
*/
void requestServer(queue_entry_t action, CLIENT *handle, int arg) {
    authRequest_t request;
    authToken_t *response;

    /* (1) */
    request.clientId = action.id;
    response = request_authorization_1(&request, handle);

    if(response->error == SUCC) {
        /* (2) */
        approveTokenRequest_t approveRequest;
        approveRequest.authToken = response->token;
        authToken_t *responseApproval;

        responseApproval = approve_request_token_1(&approveRequest, handle);

        accessRequest_t        accessRequest;
        accessTokenResponse    *accessResponse;

        accessRequest.clientId  = request.clientId;
        accessRequest.arg       = action.arg;
        accessRequest.authToken = responseApproval->token;

        /* (3) */
        accessResponse = request_access_token_1(&accessRequest, handle);
        if(accessResponse->error == REQUEST_DENIED) {
            cout << "REQUEST_DENIED" << endl;
        } else {
            if(action.arg == 0) {
                cout << approveRequest.authToken << " -> " << accessResponse->accessToken << endl;
                usersValability.erase(string(action.id));
            } else {

                /* add valability and refresh token */
                usersValability[action.id]       = accessResponse->valability;
                usersRefreshTokens[action.id]    = accessResponse->refreshToken;
                cout << approveRequest.authToken << " -> " << accessResponse->accessToken << "," << accessResponse->refreshToken << endl;
            }
        }
        
        usersAccessTokens[action.id]     = accessResponse->accessToken;
    } else {
        cout << "USER_NOT_FOUND" << endl;
    }
}

/*
    Check if the client has opted for automatic refresh of the access token
    and if its valability has got to 0, in which case an operation of
    request_access_token will be send to the server in order to compute
    the new corresponding access token.
*/
bool checkValability(char *clientId) {
    if(usersValability.find(string(clientId)) == usersValability.end())
        return false;
    if(usersValability[clientId] == 0)
        return true;
    return false;
}

/*
    Decrement client's valability if the client has opted for automatic
    refresh of the access token.
*/
void updateValability(char *clientId) {
    if(usersValability.find(string(clientId)) != usersValability.end())
        usersValability[clientId] -= 1;    
}

/*
    Function for automatically refresh access token
*/
void refreshToken(char *clientId, CLIENT *handle) {
    accessRequest_t request;
    accessTokenResponse *response;

    /*
        Prepare accessRequest_t package
    */
    request.action      = REFRESH;      /* Mark the operation as being refresh */
    request.clientId    = clientId;     /* Copy client id */
    request.authToken   = (char *)(usersRefreshTokens[clientId].c_str()); /* Copy refresh token */

    response = request_access_token_1(&request, handle);
    /* 
        Update local maps with received information
    */
    if(response->error == SUCC) {
        usersAccessTokens[clientId]     = response->accessToken;
        usersRefreshTokens[clientId]    = response->refreshToken;
        usersValability[clientId]       = response->valability;
    }
}

/*
    Function for requesting RIMDX operations to the server
*/
void requestAction(queue_entry_t action, CLIENT *handle, int arg) {
    /*
        Check if the client has requested automatic refresh access token
        and if so, check the valability of its access token. In case of
        an expired token, process the automatic refresh of the access token step.
    */
    if(checkValability(action.id)) {
        refreshToken(action.id, handle);
    }

    /*
        Compute the delegateAction_t package
    */
    delegateAction_t requestAction;
    requestAction.action = action.actionId;             /* send the action */
    requestAction.resource  = action.strarg;            /* send the resource */
    if(requestAction.action == UNKNOWN)
        requestAction.unknown_cmd = action.unknownCmd;  /* send the string command in case of UNKNOWN command */
    requestAction.token     = (char *)(usersAccessTokens[action.id].c_str()); /* send the corresponding access token */

    /*
        Send the request to the server
    */
    actionResponse_t *response;
    response = validate_delegated_action_1(&requestAction, handle);

    /* 
        Check error code and print log output 
    */
    if(response->err == PERMISSION_DENIED) {
        cout << "PERMISSION_DENIED" << endl;
        return;
    } if(response->err == TOKEN_EXPIRED)
        cout << "TOKEN_EXPIRED" << endl;
    if(response->err == PERMISSION_GRANTED)
        cout << "PERMISSION_GRANTED" << endl;
    if(response->err == RESOURCE_NOT_FOUND)
        cout << "RESOURCE_NOT_FOUND" << endl;
    if(response->err == OPERATION_NOT_PERMITTED)
        cout << "OPERATION_NOT_PERMITTED" << endl;

    /*
        Decrement client's valability with 1 operation executed
    */
    updateValability(action.id);
}

/* 
    Function called from the main client for executing the input actions.
    Based on the actionId, we separate 2 cases
        1) Request for authorization process
        2) Actions for RIMDX operations
*/
bool executeActions(CLIENT *handle) {
    while(!actions.empty()) {
        queue_entry_t action = actions.front();
        if(action.actionId == REQUEST) {
            requestServer(action, handle, action.arg);
        } else {
            requestAction(action, handle, action.arg);
        }
        actions.pop();
    }
    return true;
}


/* Function for parsing the string-format action in actionCodes enum */
actionCodes parseAction(char *action) {
    if(strcmp(action, "REQUEST") == 0)
        return REQUEST;
    if(strcmp(action, "INSERT") == 0)  
        return INSERT;
    if(strcmp(action, "MODIFY") == 0)
        return MODIFY;
    if(strcmp(action, "EXECUTE") == 0)
        return EXECUTE;
    if(strcmp(action, "DELETE") == 0)
        return DELETE;
    if(strcmp(action, "READ") == 0)
        return READ;

    return UNKNOWN;
}

/*
    Function for creating a new action in order to add it into the queue.
*/
queue_entry_t createQueueAction(char *id, char *action, char*arg) {
    queue_entry_t newEntry;

    /* client's id */
    newEntry.id         = strdup(id);

    /* transform the string format of the action in actionCodes enum */
    newEntry.actionId   = parseAction(action);

    /*  In case of REQUEST action, the 3rd argument is expected to be integer 
        0 or 1, depending on the automatic refresh of the access token 
        
        Any other command RIMDX operation expects 3rd argument a string
        corresponding to the resource in the database  */
    if(newEntry.actionId == REQUEST)
        newEntry.arg    = atoi(arg);
    else
        newEntry.strarg = strdup(arg);

    /* In case of unknown command, also keep the command action in string format */
    if(newEntry.actionId == UNKNOWN)
        newEntry.unknownCmd = strdup(action);

    return newEntry;
}

/* 
    Function for parsing input file (client.in) having the following
    format: <CLIENT_ID>,<ACTION>,<ARG>
*/
bool parseInputData(char *filename) {
    ifstream inputFile;
    string inputLine;
    inputFile.open(filename);

    /* Throw error in case of unknown file */
    if(!inputFile.is_open()) {
        cerr << "Unknown file!" << endl;
        return false;
    }

    while(inputFile >> inputLine) {  
        char *s = &inputLine[0];
        char token[] = ",";
        char *p         = strtok(s, token);     /* <CLIENT_ID> */
        char *action    = strtok(NULL, token);  /* <ACTION> */
        char *arg       = strtok(NULL, token);  /* <ARG> */
        
        actions.push(createQueueAction(p, action, arg));    /* add newly created action in the queue */ 
    }
    
    inputFile.close();

    return true;
}

void cleanUp() {
    usersAccessTokens.clear();
    usersRefreshTokens.clear();
    usersValability.clear();
}