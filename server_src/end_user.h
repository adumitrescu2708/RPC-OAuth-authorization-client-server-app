#include "../application.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include "database.h"
/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | END USER SOURCE CODE

*/

using namespace std;

/* Method used for parsing the input file */
bool parseAprovals(char *filename);

/* Method used for validating authorization tokens and adding permissions to database */
bool validateEndUser(char *clientId, char *authToken);