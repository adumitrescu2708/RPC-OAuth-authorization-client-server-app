/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | CLIENT HELPER'S SOURCE CODE

*/

#include "../application.h"
#include <string.h>
#include <queue>
#include <iostream>
#include <fstream>
#include <unordered_map> 

using namespace std;

/*
    Struct for encoding the operations described in client.in input file.

    When parsing the actions from the input file,
    each action is pushed in a queue
*/
struct queue_entry_t {
    char            *id;            /* client id */
    actionCodes     actionId;       /* action id */
    int             arg;            /* integer argument used for REQUEST operations 0 or 1 for refresh token */
    char            *strarg;        /* string argument used for resource encoding */
    char            *unknownCmd;    /* in case of UNKNOWN command, store also the entire command */
};


/* Function for parsing client.in input file */
bool parseInputData(char *filename);

/* Function for executing actions and sending them to received client */
bool executeActions(CLIENT *handle);

/* Free space */
void cleanUp();