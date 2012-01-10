/*
   Copyright (C) 2009 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _H_REDS
#define _H_REDS

#include <stdint.h>
#include <openssl/ssl.h>
#include <sys/uio.h>
#include <spice/vd_agent.h>
#include <config.h>

#if HAVE_SASL
#include <sasl/sasl.h>
#endif

#include "common/marshaller.h"
#include "common/messages.h"
#include "spice.h"
#include "red_channel.h"

#define SPICE_GNUC_VISIBLE __attribute__ ((visibility ("default")))

#if HAVE_SASL
typedef struct RedsSASL {
    sasl_conn_t *conn;

    /* If we want to negotiate an SSF layer with client */
    int wantSSF :1;
    /* If we are now running the SSF layer */
    int runSSF :1;

    /*
     * Buffering encoded data to allow more clear data
     * to be stuffed onto the output buffer
     */
    const uint8_t *encoded;
    unsigned int encodedLength;
    unsigned int encodedOffset;

    SpiceBuffer inbuffer;

    char *username;
    char *mechlist;
    char *mechname;

    /* temporary data during authentication */
    unsigned int len;
    char *data;
} RedsSASL;
#endif

struct RedsStream {
    int socket;
    SpiceWatch *watch;

    /* set it to TRUE if you shutdown the socket. shutdown read doesn't work as accepted -
       receive may return data afterward. check the flag before calling receive*/
    int shutdown;
    SSL *ssl;

#if HAVE_SASL
    RedsSASL sasl;
#endif

    SpiceChannelEventInfo info;

    /* private */
    ssize_t (*read)(RedsStream *s, void *buf, size_t nbyte);
    ssize_t (*write)(RedsStream *s, const void *buf, size_t nbyte);
    ssize_t (*writev)(RedsStream *s, const struct iovec *iov, int iovcnt);
};

struct QXLState {
    QXLInterface          *qif;
    struct RedDispatcher  *dispatcher;
};

struct TunnelWorker;
struct SpiceNetWireState {
    struct TunnelWorker *worker;
};

struct SpiceMigrateState {
    int dummy;
};

typedef struct RedsMigSpice {
    char *host;
    char *cert_subject;
    int port;
    int sport;
} RedsMigSpice;

ssize_t reds_stream_read(RedsStream *s, void *buf, size_t nbyte);
ssize_t reds_stream_write(RedsStream *s, const void *buf, size_t nbyte);
ssize_t reds_stream_writev(RedsStream *s, const struct iovec *iov, int iovcnt);
void reds_stream_free(RedsStream *s);

void reds_disable_mm_timer(void);
void reds_enable_mm_timer(void);
void reds_update_mm_timer(uint32_t mm_time);
uint32_t reds_get_mm_time(void);
void reds_set_client_mouse_allowed(int is_client_mouse_allowed,
                                   int x_res, int y_res);
void reds_register_channel(RedChannel *channel);
void reds_unregister_channel(RedChannel *channel);
int reds_get_mouse_mode(void); // used by inputs_channel
int reds_get_agent_mouse(void); // used by inputs_channel
int reds_has_vdagent(void); // used by inputs channel
void reds_handle_agent_mouse_event(const VDAgentMouseState *mouse_state); // used by inputs_channel

extern struct SpiceCoreInterface *core;

// Temporary measures to make splitting reds.c to inputs_channel.c easier
void reds_client_disconnect(RedClient *client);

// Temporary (?) for splitting main channel
typedef struct MainMigrateData MainMigrateData;
void reds_marshall_migrate_data_item(SpiceMarshaller *m, MainMigrateData *data);
void reds_fill_channels(SpiceMsgChannels *channels_info);
int reds_num_of_channels(void);
int reds_num_of_clients(void);
#ifdef RED_STATISTICS
void reds_update_stat_value(uint32_t value);
#endif

/* callbacks from main channel messages */

void reds_on_main_agent_start(void);
void reds_on_main_agent_data(MainChannelClient *mcc, void *message, size_t size);
void reds_on_main_migrate_connected(void); //should be called when all the clients
                                           // are connected to the target
void reds_on_main_receive_migrate_data(MainMigrateData *data, uint8_t *end);
void reds_on_main_mouse_mode_request(void *message, size_t size);
void reds_on_client_migrate_complete(RedClient *client);
#endif
