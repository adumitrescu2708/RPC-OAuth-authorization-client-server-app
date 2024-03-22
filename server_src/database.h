/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | DATABASE SOURCE CODE

*/

#include "../application.h"
#include <iostream>
#include <unordered_map> 
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

#define ERROR " "

/* RESOURCES */
extern vector<string>                   resources;          /* resources in database */
extern int                              resourcesCount;     /* number of resources */

/* CLIENTS & TOKENS */
extern int                              valability;         /* token's valability for standard user */
extern int                              usersCount;         /* number of users */
extern vector<string>                   usersIds;           /* list of user ids */
extern unordered_map<string, string>    users;              /* mapping user's id to user's access tokens */
extern unordered_map<string, int>       usersValability;    /* mapping user's id to valability */

/* PERMISSIONS */
extern unordered_map<string, unordered_map<string, string>> usersPermisions;    /* mapping user's id to permissions in database */


/* Method that checks if the given user id is in the list of known users ids */
bool checkUserId(char *id);


/* Method for parsing the known user's ids file */
bool parseInputIds(char *filename);


/* Method for parsing the list of resources in the database */
bool parseResources(char *filename);


/* Parses the given argument in the server main function and stores it */
void setValabilityTokens(char *token);


/*  Creates a corresponding from the client id to its authorization token and stores
    it in the users map */
void addAuthToken(char *clientId, char *token);


/*  Creates a correstponding from the client id to ids access token and adds it in
    the users map, replacing the authorization token */
void addAccessToken(char *clientId, char *accessToken);


/* Retrieves the standard valability of the user's access token */
int getValability();


/* Checks if the access token corresponds to the client id */
bool checkAccessToken(char *clientId, char *clientAccessToken);


/* Checks if the user has more availability for the current access token. */
bool checkAvailabilityToken(char *clientId);


/* Checks if the given resource is in the list of resources in the database */
bool checkResource(char *resource);


/*  Decrements the availability of the current access token of the given user
    every time a RIMDX operation is requested */
void decrementAvailability(char *clientId);


/* Sets the permission of all resources to the given client */
void setUserPermissions(char * clientId, unordered_map<string, string> perm);


/* Checks if the given action on the given resource is permitted for the given client */
bool checkPermissions(char *clientId, char *resource, actionCodes action);


/* Returns the remained availability of the current access token of the given user */
int getAvailability(char *clientId);


/* Add in the database the new access token */
void refreshDatabase(char *clientId, char *newAccessToken);


/* Retrieves the client id based on the access token */
string getId(char * token);