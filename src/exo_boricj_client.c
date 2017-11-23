/*  =========================================================================
    exo_boricj_client - Actor

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
    exo_boricj_client - Binary
@discuss
@end
*/

#include "exo_boricj_classes.h"

zmsg_t * build_request (char *req)
{
    zmsg_t *message = zmsg_new ();
    char *tok, *saveptr;
    const char *delim = ",\n";

    tok = strtok_r (req, delim, &saveptr);
    while (tok) {
        zmsg_addstr (message, tok);
        tok = strtok_r (NULL, delim, &saveptr);
    }

    return message;
}

void client_mailbox_mainloop (mlm_client_t *client, bool verbose)
{
    char buf[128];

    while (fgets(buf, sizeof(buf), stdin)) {
        // Send request.
        zmsg_t *message = build_request (buf);
        if (mlm_client_sendto (client, "exo-boricj", "bottles", NULL, 1000, &message) != 0) {
            if (verbose) {
                zsys_error ("mlm_client_sendto () failed.");
            }
            zmsg_destroy (&message);
            continue;
        }

        // Receive lyrics.
        message = mlm_client_recv (client);
        if (message == NULL) {
            if (verbose) {
                zsys_error ("mlm_client_recv () failed.");
            }
            continue;
        }

        // Print out lyrics.
        char *ret;
        while ((ret = zmsg_popstr (message))) {
            zsys_info (ret);
            zstr_free (&ret);
        }

        zmsg_destroy (&message);
    }
}

void
dump_stream_bottles (zsock_t *pipe, void* args)
{
    const char *endpoint = "ipc://@/malamute";

    // Create client.
    mlm_client_t *client = mlm_client_new ();
    if (client == NULL) {
        zsys_error ("mlm_client_new () for dumper failed.");
    }

    // Connect client.
    char name[64];
    snprintf(name, sizeof(name), "exo_boric_client-%d-dumper", (int)getpid());
    int rv = mlm_client_connect (client, endpoint, 1000, name);
    if (rv == -1) {
        mlm_client_destroy (&client);
        zsys_error ( "mlm_client_connect () for dumper failed.");
    }

    if (mlm_client_set_consumer (client, EXO_BORICJ_STREAM_LYRICS, ".*") != 0) {
        mlm_client_destroy (&client);
        zsys_error ("mlm_client_set_consumer () failed.");
    }

    zpoller_t *poller = zpoller_new (pipe, mlm_client_msgpipe (client), NULL);
    zsock_signal (pipe, 0);
    zsys_debug ("actor ready");

    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, -1);
        if (which == pipe) {
            zmsg_t *message = zmsg_recv (pipe);
            char *actor_command = zmsg_popstr (message);
            //  $TERM actor command implementation is required by zactor_t interface
            if (streq (actor_command, "$TERM")) {
                zstr_free (&actor_command);
                zmsg_destroy (&message);
                break;
            }
            zstr_free (&actor_command);
            zmsg_destroy (&message);
            continue;
        }

        // Receive lyrics.
        zmsg_t *message = mlm_client_recv (client);
        if (message == NULL) {
            zsys_debug ("interrupted");
            break;
        }

        // Print out lyrics.
        char *ret;
        while ((ret = zmsg_popstr (message))) {
            zsys_info (ret);
            zstr_free (&ret);
        }

        zmsg_destroy (&message);
    }

    mlm_client_destroy (&client);
    zpoller_destroy (&poller);
}

void client_stream_mainloop (mlm_client_t *client, bool verbose)
{
    char buf[128];

    if (mlm_client_set_producer (client, EXO_BORICJ_STREAM_REQUEST) != 0) {
        zsys_error ("mlm_client_set_producer () failed.");
        return;
    }

    while (fgets(buf, sizeof(buf), stdin)) {
        // Send request.
        zmsg_t *message = build_request (buf);
        if (mlm_client_send (client, "bottles", &message) != 0) {
            if (verbose) {
                zsys_error ("mlm_client_sendto () failed.");
            }
            zmsg_destroy (&message);
        }
    }
}

int main (int argc, char *argv [])
{
    bool verbose = false;
    bool stream = false;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("exo-boricj-main [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("  --stream / -s          operate in stream mode");
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--stream")
        ||  streq (argv [argn], "-s"))
            stream = true;
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }

    const char *endpoint = "ipc://@/malamute";

    // Create client.
    mlm_client_t *client = mlm_client_new ();
    if (client == NULL) {
        zsys_error ("mlm_client_new () failed.");
        return EXIT_FAILURE;
    }

    // Connect client.
    char name[64];
    snprintf(name, sizeof(name), "exo_boric_client-%d", (int)getpid());
    int rv = mlm_client_connect (client, endpoint, 1000, name);
    if (rv == -1) {
        mlm_client_destroy (&client);
        zsys_error (
                "mlm_client_connect (endpoint = '%s', timeout = '%d', address = '%s') failed.",
                endpoint, 1000, "fty-example");
        return EXIT_FAILURE;
    }

    if (stream) {
        zsys_info("operating in stream mode");
        zactor_t *dumper = zactor_new (dump_stream_bottles, (void *) endpoint);
        client_stream_mainloop (client, verbose);
        zactor_destroy(&dumper);
    }
    else {
        zsys_info("operating in mailbox mode");
        client_mailbox_mainloop (client, verbose);
    }

    mlm_client_destroy (&client);

    return EXIT_SUCCESS;
}
