/*
    |   Credits: Dumitrescu Alexandra - 343 C1
*/


/*
    |   Possible error codes
*/
enum erroCodes {
    SUCC,                       /* Marks the operation as being successful */
    USER_NOT_FOUND,             /* Used when the user id is not found in the server's input list */
    REQUEST_DENIED,             /* Used for invalid/unsigned authorization token */
    PERMISSION_DENIED,          /* Used for unknown access token */
    TOKEN_EXPIRED,              /* Used for failing action due to expired tokens */
    RESOURCE_NOT_FOUND,         /* Used when the resource is not found in the database */
    OPERATION_NOT_PERMITTED,    /* Marks when the user doesn't have the required permissions */
    PERMISSION_GRANTED          /* Marks an operation as being permited */
};

/*
    |   Possible actions
*/
enum actionCodes {
    REQUEST,        /* Used for input request */
    MODIFY,         /* Modify */
    EXECUTE,        /* Execute */
    DELETE,         /* Delete */
    INSERT,         /* Insert */
    READ,           /* Read */
    REFRESH,        /* used internally for refresh purposes in regenerating access token */
    UNKNOWN         /* any other input command */
};

/*
    |   Server's response to request authorization process 
*/

struct authToken_t {
    string token<>;             /* authorization token or NULL in case of error */
    erroCodes error;            /* SUCC or USER_NOT_FOUND if the user's id is not found in database */
};


/*
    |   Server's response to request access token 
*/
struct accessTokenResponse {
    string accessToken<>;          /* access token */
    string refreshToken<>;         /* refresh token */
    int valability;                /* standard valability for each user */
    erroCodes error;               /* SUCC or REQUEST_DENIED in case of an unsigned/invalid authorization token */
};

/*
    |   Server's response to action delegations
*/
struct actionResponse_t {
    erroCodes err;                  /* PERMISSION_DENIED, TOKEN_EXPIRED, RESOURCE_NOT_FOUND, OPERATION_NOT_PERMITTED or PERMISSION_GRANTED */
};


/*
    | Client's authorization struct to server
*/
struct authRequest_t {
    string clientId<>;          /* client id */
};


/*
    | Client's approve struct to end user
*/
struct approveTokenRequest_t {
    string authToken<>;         /* authorization token */
};


/*
    | Client's access token request struct to server
*/
struct accessRequest_t {
    string clientId<>;          /* client id */
    string authToken<>;         /* authorization token */
    int arg;                    /* 0 or 1 depending on the automatic refresh token prefference */
    actionCodes action;         /* encoding REQUEST or REFRESH */
};


/*
    | Client's action request struct to server
*/
struct delegateAction_t {
    actionCodes action;         /* encoding RIMDX operations */
    string resource<>;          /* resource in database */
    string token<>;             /* access token */
    string unknown_cmd<>;       /* string command in case of an UNKNOWN action code */
};



program APPLICATION_PROG {
    version APPLICATION_VERSION {
        authToken_t             request_authorization(authRequest_t request) = 1;
        accessTokenResponse     request_access_token(accessRequest_t request) = 2;
        authToken_t             approve_request_token(approveTokenRequest_t request) = 3;
        actionResponse_t        validate_delegated_action(delegateAction_t action) = 4;
    } = 1;
} = 1;