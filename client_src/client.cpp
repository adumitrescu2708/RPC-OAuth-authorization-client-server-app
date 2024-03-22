/*
    | Credits:  Dumitrescu Alexandra - 343 C1
    | From:     Nov 2023

    | CLIENT'S SOURCE CODE

*/

/*  
    Client uses client_helper source code for parsing input and
    executing input commands
*/
#include "client_helper.h"

#define NUM_ARGV 3

using namespace std;


int main(int argc, char **argv) {

    /* Check possible bad usage */
    if(argc != NUM_ARGV) {
        cerr << "Bad usage! Try: ./client <server_address> <input_file> !" << endl;
        return -1;
    }

    /* Send argv to client helper to parse input data */
    parseInputData(argv[2]);

    /* Create connection to server */
    CLIENT *handle;
	handle=clnt_create(
		argv[1],
		APPLICATION_PROG,
		APPLICATION_VERSION,
		"tcp");

    /* Defensive checks */
    if(handle == NULL) {
		cerr << "Cannot connect!" << endl;
		return -1;
	}

    /* execute actions described in client.in */
    executeActions(handle);
    cleanUp();

    return 0;  
}