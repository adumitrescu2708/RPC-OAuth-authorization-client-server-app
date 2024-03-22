# OAuth Client Server Application - SPRC Tema 1

> Name: *Dumitrescu Alexandra*
>
> Date: *November 2023*

## Content
<ol>
  <li>Project structure</li>
  <li>Observations</li>
  <li>Implementation
    <ol>
      <li>Common Interface</li>
      <li>Server & Database Implementation</li>
      <li>Client Implementation</li>
      <li>Automatic refresh of the access tokens</li>
    </ol>
  </li>
</ol>


## Project structure
<pre>
|_ application.x                    -> describes the common  
|                                   interface between client and server  
|_ application_svc_modified.c       -> the auto generated application_svc.c
|                                   has been modified in order to retrieve the argv 
|_ Makefile  
|_ Readme.md
|_ server_src/  
    |_ database.cpp & database.h    -> implements the database logic
    |                               and exposes useful methods to server  
    |_ end_user.cpp & end_user.h    -> implements the signing of authorization
    |                               tokens and exposes methods to server  
    |_ server.cpp  
|_ client_src/  
    |_ client.cpp                             -> connects to server and parses input  
    |_ client_helper.c & client_helper.h      -> implements the logic of executing the commands  
</pre>


## Observations
1. I modified the auto generated application_svc.c file in order to retrieve the input specified files  
in the cmd line argument and send them to the databse.  

2. I modified the expected output files by adding a newline at the end of each file.

3. I added in Makefile a rule of overwriting the auto generated application_svc.c file with  
the modified one.

4. For more details, check comments in the .h & source files.

## Implementation
### Common Interface
The common interface between the client and the user is described in _application.x_ file.  
The API consists of 4 methods:  

> *authToken_t request_authorization(authRequest_t request)*  
>> CLIENT: sends a request in an authRequest_t structure containing a string for client id  
>>
>> SERVER: returns an authToken_t structure response containing one string for the token  
>> or NULL in case of an error and an error code (which can be either SUCC or USER_NOT_FOUND if  
>> the received client id is not in the list of server-known users)


> *accessTokenResponse request_access_token(accessRequest_t request)*  
>> CLIENT: sends a request in an accessRequest_t structure containing a string for client id,  
>> a string for the earlier received authorization token, an integer argument for 0 or 1  
>> depending on wether the user chooses automatic refresh of the access token or not  
>> and an action code which can be either REQUEST or REFRESH.
>>
>> SERVER: returns an accessTokenResponse structure response containing one string for the  
>> access token, one for the refresh token (which is NULL if the client has sent argument 0),  
>> an error code (which can be SUCC or REQUEST_DENIED if the end user has not signed the  
>> authorization token)


> *authToken_t approve_request_token(approveTokenRequest_t request)*  
>> CLIENT: sends a request in an approveTokenRequest_t structure containing a string    
>> for authorization token received from the server.
>>
>> END-USER: returns an authToken_t structure response containing one string for the  
>> authorization token (which can be signed or unsigned). In case of a signed authorization  
>> token, the end user would place at the beginning of the token the string "SIGNED" and  
>> would send to the database the permissions asociated to the user.  


> *actionResponse_t validate_delegated_action(delegateAction_t action)*  
>> CLIENT: sends a request in an actionResponse_t structure containing an action code  
>> specific for the RIMDX operation, a string for the target resource in database, a      
>> string for the access token and a string argument in case of an unknown command.
>>
>> SERVER: returns an actionResponse_t structure response containing one error code  
>> (which can be PERMISSION_DENIED (if the access token is not associated to the user)  
>> TOKEN_EXPIRED (if the access token has expired), RESOURCE_NOT_FOUND (the resource is not   
>> in the database), OPERATION_NOT_PERMITTED (the client doesn't have the specific permission  
>> on the resource) or PERMISSION_GRANTED)  

I have also added a common interface for the error codes and allowed actions, described  
with the enums erroCodes and actionCodes.

### Server Implementation


> *authToken_t request_authorization(authRequest_t request)*  
>> At the start of the server, an input file describing the known users ids is being  
>> sent to the database. During the request_authorization operation, the server  
>> interogates the database to check if the received client id is known, in which  
>> case an authorization token is being generated and sent to the user with error code SUCC.  
>> If the user is not known, the server will send NULL token and USER_NOT_FOUND error.


> *authToken_t approve_request_token(approveTokenRequest_t request)*  
>> Receives from client the authorization token, interogates the database to obtain the client   
>> id and sends <auth_token, client_id> to end user. The end user will read a line of permissions    
>> and send the mapping from resource to permission to the database for user described by client_id.  
>> Will send back to the user a signed auth token (appending string "SIGNED" to the auth token)  
>> or send the token back the same if not signed.  


> *accessTokenResponse request_access_token(accessRequest_t request)*  
>> The server will firstly check if the received action is a refresh action. If so,  
>> it will re-generate new access token and refresh token and update the valability  
>> and the information in the databased. If it is not a refresh action, then it is a  
>> request action. During a request, the server would check if the received authorization  
>> token is signed (and throw REQUEST_DENIED if not) and then compute refresh and  
>> access tokens. 

> *actionResponse_t validate_delegated_action(delegateAction_t action)*  
>> The server would simply validate the checkings previously described.

In the database we proposed 3 unordered_map for fast queries: one mapping  
_userId->accessToken_, one for _userId->permissions_ and one for __userId->valability.  
Apart from this, I also used vectors for parsing the server-known userIds and resources.

### Client Implementation
The client uses a client helper where we keep trask of the following:  
one map from userId to refreshToken, from userId to accessToken, from userId to valability.  
Before each action (RIMDX) request, the client helper checks the valability of the given  
user and if the user has opted for automatic refresh, sends an refresh request as mentioned  
in the following chapter.

### Automatic refresh of the access tokens 
If the client has sent argument 1 to _request_access_token_ then the server would generate  
a refresh token, but it will not keep track of them. In the client_helper logic,  
there is a _usersValability_ unordered_map that keeps track of the number of operations  
left for each user that has opted for auto refresh. Before executing a _validate_delegated_action_  
request, the client helper will check if the user has opted for auto refresh and check  
if there are more operations left in _usersValability_. If not, it would generate a _request_access_token_  
to the server, marking it as REFRESH action and sending the refresh token as an authorization token.


