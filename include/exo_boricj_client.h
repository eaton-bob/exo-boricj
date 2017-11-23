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

#ifndef EXO_BORICJ_CLIENT_H_INCLUDED
#define EXO_BORICJ_CLIENT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new exo_boricj_client
EXO_BORICJ_EXPORT exo_boricj_client_t *
    exo_boricj_client_new (void);

//  Destroy the exo_boricj_client
EXO_BORICJ_EXPORT void
    exo_boricj_client_destroy (exo_boricj_client_t **self_p);

//  Self test of this class
EXO_BORICJ_EXPORT void
    exo_boricj_client_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
