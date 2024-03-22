/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | SERVER SOURCE CODE

*/
#include <bits/stdc++.h>
#include <rpc/rpc.h>
#include "../application.h"
#include "database.h"
#include "end_user.h"
#include "../token.h"

#define SIGNED "SIGNED"

/*
    1. Check if the received action is a refresh action.
        1.1 In this case, the user's auth token is the refresh token.
        1.2 Generate new access token and refresh token
        1.3 Update database entries (reset valability and update previous tokens)
    2. Check if the auth token is signed
        2.1 If the token is not signed return REQUEST_DENIED error
    3. Remove the SIGNED string from the beginning of the auth token
    4. Based on the auth token, compute the access token
    5. Check the received argument. If set to 0 (meaning that the client has not
       opted for the automatic refresh of the auth token) do not generate a refresh
       token. If set to 1, generate a refresh token based on the access token.
    6. Send the access token to be stored in the database
*/
accessTokenResponse * request_access_token_1_svc(accessRequest_t *request, struct svc_req *sv) {
    char *clientId  = request->clientId;
    char *authToken = request->authToken;
    int refresh     = request->arg;

    static accessTokenResponse response;
    
    /* (1) */
    if(request->action == REFRESH) {
        cout << "BEGIN " << clientId << " AUTHZ REFRESH" << endl;
        /* (1.2) */
        char *newAccessToken    = generate_access_token(strdup(authToken));
        char *newRefreshToken   = generate_access_token(strdup(newAccessToken));

        cout << "  AccessToken = " << newAccessToken << endl;
        cout << "  RefreshToken = " << newRefreshToken << endl;
        /* (1.3) */
        refreshDatabase(clientId, newAccessToken);

        response.accessToken    = newAccessToken;
        response.valability     = getValability();
        response.error          = SUCC;
        response.refreshToken   = newRefreshToken;

        return &response;
    }

    /* (2) */
    if(strncmp(authToken, SIGNED, strlen(SIGNED)) != 0) {
        response.error          = REQUEST_DENIED;
        response.accessToken    = strdup(" ");
        response.refreshToken   = strdup(" ");
        response.valability     = -1;
        return &response;
    }

    /* (3) */
    char *authTokenNoSign = strdup(authToken + strlen(SIGNED));

    /* (4) */
    char *accessToken   = generate_access_token(strdup(authTokenNoSign));

    /* (5) */
    char *refreshToken;
    if(refresh == 1)
        refreshToken = generate_access_token(strdup(accessToken));
    else
        refreshToken = strdup(" ");

    cout << "  AccessToken = " << accessToken << endl;
    if(refresh == 1)
        cout << "  RefreshToken = " << refreshToken << endl;

    /* (6) */
    addAccessToken(clientId, accessToken);

    response.error          = SUCC;
    response.accessToken    = accessToken;
    response.refreshToken   = refreshToken;
    response.valability     = getValability();

    return &response;
}

/*
    1. Based on the received authorization token, retrieve from the database the client id
    2. Send to the end user the client id and the authorization token (the client id is
       used for knowing where to place the permissions in the database)
*/
authToken_t * approve_request_token_1_svc(approveTokenRequest_t *request, struct svc_req *sv) {
    /* (1) */
    char *clientId          = (char *)(getId(request->authToken).c_str());
    char *authToken         = request->authToken;
    char *signedAuthToken   = authToken;

    static authToken_t response;

    /*(2) */
    if(validateEndUser(clientId, authToken)) {
        response.token = strcat(strdup(SIGNED), signedAuthToken);
    } else {
        response.token = signedAuthToken;
    }

    return &response;
}

/*
    1. Interogate the databse in order to check if the received client id is known.
    2. Send USER_NOT_FOUND error and NULL token if the client is not known.
    3. Generate authorization token and send back to user with SUCC error. Store
       in the database the authorization token for the client
*/
authToken_t * request_authorization_1_svc(authRequest_t *request, struct svc_req *sv) {
    static authToken_t response;
    char *clientId = request->clientId;

    cout << "BEGIN " << clientId << " AUTHZ" << endl;

    /* (1) */
    if(checkUserId(clientId)) {
        /* (3) */
        response.error  = SUCC;
        char *token     = generate_access_token(strdup(request->clientId));

        addAuthToken(request->clientId, token);
        cout << "  RequestToken = " << token << endl;
        response.token = token;
    } else {
        /* (2) */
        response.error  = USER_NOT_FOUND;
        char *token     = NULL;
    }

    return &response;
}

/*
    Utility function for printing the log message.
*/
void printErrorLog(actionCodes action, char *resource, char *clientToken, int availability, bool error) {
    if(error) {
        cout << "DENY (";
    } else {
        cout << "PERMIT (";
    }
    
    if(action == READ) {
        cout << "READ,";
    }
    if(action == INSERT) {
        cout << "INSERT,";
    }
    if(action == MODIFY) {
        cout << "MODIFY,";
    }
    if(action == DELETE) {
        cout << "DELETE,";
    }
    if(action == EXECUTE) {
        cout << "EXECUTE,";
    }
    cout << resource << "," << clientToken <<"," << availability << ")" << endl;
}

/*
    Utility function for special case of unknown commands
*/
void printUnknownErrorLog(char *unknown, char *resource, char *clientId, int availability) {
    cout << "DENY (" << unknown << "," << resource << "," << clientId << "," << availability << ")" << endl;
}

/*
    1. Obtain the client id from the database based on the received access token
       If the access token is NOT in the database then the clientId will be set to empty string
    2. Check if the client id and access tokens are in the database
    3. Check availability of the client
    4. Check if the resource is in the database
    5. Check if the received command is unknown
    6. Check the permissions
*/
actionResponse_t * validate_delegated_action_1_svc(delegateAction_t *request, struct svc_req *sv) {
    char *clientAccessToken = request->token;
    char *resource          = request->resource;
    actionCodes action      = request->action;
    /* (1) */
    char *clientId          = (char *)(getId(clientAccessToken).c_str());

    static actionResponse_t response;

    /* (2) */
    if(!checkAccessToken(clientId, clientAccessToken)) {
        printErrorLog(action, resource, clientAccessToken, 0, true);
        response.err = PERMISSION_DENIED;
        return &response;
    }

    /* (3) */
    if(!checkAvailabilityToken(clientId)) {
        char empty[] = "";
        printErrorLog(action, resource, empty, getAvailability(clientId), true);
        response.err = TOKEN_EXPIRED;
        return &response;
    }

    decrementAvailability(clientId);

    /* (4) */
    if(!checkResource(resource)) {
        printErrorLog(action, resource, clientAccessToken, getAvailability(clientId), true);
        response.err = RESOURCE_NOT_FOUND;
        return &response;        
    }

    /* (5) */
    if(!checkPermissions(clientId, resource, action)) {
        if(action != UNKNOWN)
            printErrorLog(action, resource, clientAccessToken, getAvailability(clientId), true);
        else
            printUnknownErrorLog(request->unknown_cmd, resource, clientAccessToken, getAvailability(clientId));
        response.err = OPERATION_NOT_PERMITTED;
        return &response;           
    }

    printErrorLog(action, resource, clientAccessToken, getAvailability(clientId), false);
    response.err = PERMISSION_GRANTED;
    return &response;
}
