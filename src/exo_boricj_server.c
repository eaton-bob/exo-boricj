/*  =========================================================================
    exo_boricj_server - Actor

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
    exo_boricj_server - Actor
@discuss
@end
*/

#include "exo_boricj_classes.h"

//  Structure of our class

struct _exo_boricj_server_t {
    int filler;     //  Declare class properties here
};

void
s_reply (mlm_client_t *client, zmsg_t **reply)
{
    // Send lyrics back by mailbox.
    if (streq (mlm_client_command (client), "MAILBOX DELIVER")) {
        int rv = mlm_client_sendto (client, mlm_client_sender (client), "beer-delivery", NULL, 1000, reply);
        if (rv != 0) {
            zsys_error ("mlm_client_sendto (subject = '%s') failed");
        }
    }
    // Send lyrics back through stream.
    else if (streq (mlm_client_command (client), "STREAM DELIVER"))  {
        int rv = mlm_client_send (client, "beer-delivery", reply);
        if (rv != 0) {
            zsys_error ("mlm_client_send (subject = '%s') failed");
        }
    }
    else {
        zsys_warning ("Unknown malamute pattern: '%s'. Message subject: '%s', sender: '%s'.",
                    mlm_client_command (client), mlm_client_subject (client), mlm_client_sender (client));
    }
}

void
s_handle (mlm_client_t *client, zmsg_t *message, int had_beers)
{
    assert (client);
    zmsg_t *reply_message = zmsg_new();
    const char *subject = mlm_client_subject (client);

    // Process lyrics request.
    if (streq (subject, "bottles")) {
        zsys_debug ("Processing %d bottles of beer for %s", zmsg_size (message), mlm_client_sender (client));
        while (zmsg_size (message) > 0) {
            int bottles;
            char *payload = zmsg_popstr (message);

            if ((sscanf (payload, "%d", &bottles) == 1) && (bottles >= 0) && (bottles < 100) ) {
                if (had_beers && (bottles > had_beers) && (bottles < 100 - had_beers)) {
                    bottles += (rand() / (float)RAND_MAX - 0.5) * 2 * had_beers;
                }
                char buf[128];

                // Write first lyric.
                snprintf (buf, sizeof(buf), "%d bottles of beer on the wall, %d bottles of beer",
                         bottles, bottles
                );
                zmsg_addstr (reply_message, buf);

                // Write second lyric.
                snprintf (buf, sizeof(buf), "take one down, pass it around, %d bottles of beer on the wall",
                         bottles-1
                );
                zmsg_addstr (reply_message, buf);
            }
            else {
                zsys_warning ("Invalid bottles of beer payload in message");
            }

            zstr_free (&payload);
        }
        s_reply (client, &reply_message);
    }
    else {
        zsys_warning ("Invalid subject in message");
    }

    zmsg_destroy (&reply_message);
}

//  --------------------------------------------------------------------------
//  Create a new exo_boricj_server

exo_boricj_server_t *
exo_boricj_server_new (void)
{
    exo_boricj_server_t *self = (exo_boricj_server_t *) zmalloc (sizeof (exo_boricj_server_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the exo_boricj_server

void
exo_boricj_server_destroy (exo_boricj_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        exo_boricj_server_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Actor function

void
exo_boricj_server (zsock_t *pipe, void* args)
{
    int had_beers = 0;
    const char *endpoint = (const char *) args;
    zsys_debug ("endpoint: %s", endpoint);

    // Create client.
    mlm_client_t *client = mlm_client_new ();
    if (client == NULL) {
        zsys_error ("mlm_client_new () failed.");
        return;
    }

    // Connect client.
    int rv = mlm_client_connect (client, endpoint, 1000, "exo-boricj");
    if (rv == -1) {
        mlm_client_destroy (&client);
        zsys_error (
                "mlm_client_connect (endpoint = '%s', timeout = '%d', address = '%s') failed.",
                endpoint, 1000, "fty-example");
        return;
    }

    // Set up streaming
    rv = mlm_client_set_consumer (client, EXO_BORICJ_STREAM_REQUEST, ".*");
    if (rv == -1) {
        mlm_client_destroy (&client);
        zsys_error (
                "mlm_client_set_consumer (stream = '%s', pattern = '%s') failed.",
                EXO_BORICJ_STREAM_REQUEST, ".*");
        return;
    }

    rv = mlm_client_set_producer (client, EXO_BORICJ_STREAM_LYRICS);
    if (rv == -1) {
        mlm_client_destroy (&client);
        zsys_error ("mlm_client_set_consumer (stream = '%s') failed.", EXO_BORICJ_STREAM_LYRICS);
        return;
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
            else
            if (streq (actor_command, "drink_beer")) {
                zsys_warning ("server drank a beer");
                had_beers++;
            }
            zstr_free (&actor_command);
            zmsg_destroy (&message);
            continue;
        }

        zmsg_t *message = mlm_client_recv (client);
        if (message == NULL) {
            zsys_debug ("interrupted");
            break;
        }

        s_handle (client, message, had_beers);
        zmsg_destroy (&message);
    }

    mlm_client_destroy (&client);
    zpoller_destroy (&poller);
}

//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

static void
exo_boricj_server_test_build_request (zmsg_t *message, int i)
{
    char buf[8];
    snprintf (buf, sizeof(buf), "%d", i);
    zmsg_addstr (message, buf);
}

static void
exo_boricj_server_test_check_response (zmsg_t *message, int i)
{
    char buf[128];
    char *ret;

    assert(ret = zmsg_popstr (message));
    snprintf (buf, sizeof(buf), "%d bottles of beer on the wall, %d bottles of beer", i, i);
    assert (streq (ret, buf));
    zstr_free (&ret);

    assert(ret = zmsg_popstr (message));
    snprintf (buf, sizeof(buf), "take one down, pass it around, %d bottles of beer on the wall", i-1);
    assert (streq (ret, buf));
    zstr_free (&ret);
}

void
exo_boricj_server_test (bool verbose)
{
    printf (" * exo_boricj_server_test: ");

    // @selftest
    static const char* endpoint = "inproc://exo-nico-server-test";

    // Set up broker.
    zactor_t *server = zactor_new (mlm_server, (void*)"Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (server, "VERBOSE");

    // Set up exo_boricj server.
    zactor_t *example_server = zactor_new (exo_boricj_server, (void *) endpoint);
    assert (example_server);

    // Set up client.
    mlm_client_t * mailBoxClient = mlm_client_new();
    assert (mailBoxClient);
    assert (mlm_client_connect (mailBoxClient, endpoint, 1000, "mailbox") == 0);
    mlm_client_t * streamClient = mlm_client_new();
    assert (streamClient);
    assert (mlm_client_connect (streamClient, endpoint, 1000, "stream") == 0);
    assert (mlm_client_set_producer (streamClient, EXO_BORICJ_STREAM_REQUEST) == 0);
    assert (mlm_client_set_consumer (streamClient, EXO_BORICJ_STREAM_LYRICS, ".*") == 0);

    int j;
    // Test in two phases: first by mailbox, then through stream.
    for (int type = 0; type < 2; type++) {
        // Play the song.
        for (int i = 99; i >= 0; i -= j) {
            j = (i < 3) ? 1 : i/2;

            zmsg_t *message = zmsg_new ();
            for (int k = i; k > i-j; k--) {
                exo_boricj_server_test_build_request (message, k);
            }

            if (type == 0) {
                // Request the full lyrics by mailbox.
                assert (mlm_client_sendto (mailBoxClient, "exo-boricj", "bottles", NULL, 1000, &message) == 0);
                assert (message = mlm_client_recv (mailBoxClient));
                assert (streq(mlm_client_subject (mailBoxClient), "beer-delivery"));
            }
            else {
                // Request the full lyrics through stream.
                assert (mlm_client_send (streamClient, "bottles", &message) == 0);
                assert (message = mlm_client_recv (streamClient));
                assert (streq(mlm_client_subject (streamClient), "beer-delivery"));
            }

            for (int k = i; k > i-j; k--) {
                exo_boricj_server_test_check_response (message, k);
            }

            zmsg_destroy (&message);
        }
    }

    mlm_client_destroy(&mailBoxClient);
    mlm_client_destroy(&streamClient);
    zactor_destroy (&example_server);
    zactor_destroy (&server);

    // @end
    printf ("OK\n");
}
