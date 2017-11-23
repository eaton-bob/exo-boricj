/*  =========================================================================
    exo_boricj_main - Binary

    Copyright (C) 2014 - 2017 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/*
@header
    exo_boricj_main - Binary
@discuss
@end
*/

#include "exo_boricj_classes.h"

int main (int argc, char *argv [])
{
    bool verbose = false;
    int had_beers = 0;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("exo-boricj [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("  --drink / -d           the server drinks a beer");
            return 0;
        }
        else
        if (streq (argv [argn], "--drink")
        ||  streq (argv [argn], "-d"))
            had_beers++;
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    //  Insert main code here
    if (verbose)
        zsys_info ("exo_boricj - Binary");
    zsys_info("starting exo_boricj_server");
    const char *endpoint = "ipc://@/malamute";
    zactor_t *boricj_server = zactor_new (exo_boricj_server, (void *) endpoint);

    while (had_beers--) {
        zstr_send (boricj_server, "drink_beer");
    }

    //  Accept and print any message back from server
    //  copy from src/malamute.c under MPL license
    while (true) {
        char *message = zstr_recv (boricj_server);
        if (message) {
            if (verbose) {
                puts (message);
            }
            free (message);
        }
        else {
            puts ("interrupted");
            break;
        }
    }
    zactor_destroy (&boricj_server);
    return EXIT_SUCCESS;
}
