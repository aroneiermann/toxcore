/* group_chats.h
 *
 * An implementation of massive text only group chats.
 *
 *
 *  Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GROUP_CHATS_H
#define GROUP_CHATS_H

#include <stdbool.h>

typedef struct Messenger Messenger;

#define TIME_STAMP_SIZE (sizeof(uint64_t))
#define MAX_GC_PACKET_SIZE 65507
#define MAX_GC_NICK_SIZE 128
#define MAX_GC_TOPIC_SIZE 512
#define MAX_GC_MESSAGE_SIZE 1368
#define MAX_GC_PART_MESSAGE_SIZE 128

#define GROUP_CLOSE_CONNECTIONS 6
#define GROUP_PING_INTERVAL 5
#define BAD_GROUPNODE_TIMEOUT 60

// CERT_TYPE + INVITEE + TIME_STAMP_SIZE + INVITEE_SIGNATURE + INVITER + TIME_STAMP_SIZE + INVITER_SIGNATURE
#define INVITE_CERTIFICATE_SIGNED_SIZE (1 + EXT_PUBLIC_KEY + TIME_STAMP_SIZE + SIGNATURE_SIZE + EXT_PUBLIC_KEY + TIME_STAMP_SIZE + SIGNATURE_SIZE)
#define SEMI_INVITE_CERTIFICATE_SIGNED_SIZE (1 + EXT_PUBLIC_KEY + TIME_STAMP_SIZE + SIGNATURE_SIZE)
// CERT_TYPE + TARGET + SOURCE + TIME_STAMP_SIZE + SOURCE_SIGNATURE
#define COMMON_CERTIFICATE_SIGNED_SIZE (1 + EXT_PUBLIC_KEY + EXT_PUBLIC_KEY + TIME_STAMP_SIZE + SIGNATURE_SIZE)

#define MAX_CERTIFICATES_NUM 5

enum {
    GC_INVITE,
    GC_BAN,
    GC_OP_CREDENTIALS
} GROUP_CERTIFICATE;

enum {
    GR_FOUNDER = 1,
    GR_OP = 2,
    GR_USER = 4,
    GR_HUMAN = 8,
    GR_ELF = 16,
    GR_DWARF = 32
} GROUP_ROLE;

enum {
    GS_NONE,
    GS_ONLINE,
    GS_OFFLINE,
    GS_AWAY,
    GS_BUSY,
    GS_INVALID
} GROUP_STATUS;

enum {
    GM_PING,
    GM_STATUS,
    GM_NEW_PEER,
    GM_CHANGE_NICK,
    GM_CHANGE_TOPIC,
    GM_MESSAGE,
    GM_PRVT_MESSAGE,
    GM_OP_ACTION,
    GM_PEER_EXIT
} GROUP_MESSAGE;

typedef struct {
    IP_Port     ip_port;

    uint8_t     client_id[EXT_PUBLIC_KEY];

    uint8_t     invite_certificate[INVITE_CERTIFICATE_SIGNED_SIZE];
    uint8_t     common_certificate[COMMON_CERTIFICATE_SIGNED_SIZE][MAX_CERTIFICATES_NUM];
    uint32_t    common_cert_num;

    uint8_t     nick[MAX_GC_NICK_SIZE];
    uint16_t    nick_len;

    bool        banned;
    uint64_t    banned_time;

    uint8_t     status;
    bool        ignore;

    bool        verified; // is peer verified, e.g. was invited by verified peer. Recursion. Problems?

    uint64_t    role;

    uint64_t    last_update_time; // updates when nick, role, verified, ip_port change or banned
    uint64_t    last_rcvd_ping;
} GC_GroupPeer;

// TODO shouldn't be neccessarry
typedef struct {
    uint8_t     client_id[EXT_PUBLIC_KEY];
    IP_Port     ip_port;
} GC_PeerAddress;

typedef struct {
    uint8_t     client_id[EXT_PUBLIC_KEY];
    uint64_t    role;    
} GC_ChatOps;

// For founder needs
typedef struct {
    uint8_t     chat_public_key[EXT_PUBLIC_KEY];
    uint8_t     chat_secret_key[EXT_SECRET_KEY];
    uint64_t    creation_time;

    GC_ChatOps   *ops;
} GC_ChatCredentials;

typedef struct GC_Announce GC_Announce;

typedef struct GC_Chat {
    Networking_Core *net;
    uint32_t hash_id;   /* 32-bit hash of self_public_key */

    uint8_t     self_public_key[EXT_PUBLIC_KEY];
    uint8_t     self_secret_key[EXT_SECRET_KEY];
    uint8_t     invite_key[EXT_PUBLIC_KEY]; /* Key used to join the chat */
    uint8_t     self_invite_certificate[INVITE_CERTIFICATE_SIGNED_SIZE];
    uint8_t     self_common_certificate[MAX_CERTIFICATES_NUM][COMMON_CERTIFICATE_SIGNED_SIZE];
    uint32_t    self_common_cert_num;

    GC_GroupPeer   *group;
    GC_PeerAddress  close[GROUP_CLOSE_CONNECTIONS];

    uint32_t    numpeers;
    int         groupnumber;

    uint8_t     self_nick[MAX_GC_NICK_SIZE];
    uint16_t    self_nick_len;
    uint64_t    self_role;
    uint8_t     self_status; // TODO: enum

    uint8_t     chat_public_key[EXT_PUBLIC_KEY];
    uint8_t     founder_public_key[EXT_PUBLIC_KEY]; // not sure about it, invitee somehow needs to check it
    uint8_t     topic[MAX_GC_TOPIC_SIZE];
    uint16_t    topic_len;

    uint64_t    last_synced_time;
    uint64_t    last_sent_ping_time;

    GC_ChatCredentials *credentials;

    uint32_t message_number;
    
    bool joined;
    bool joining;
} GC_Chat;

typedef struct GC_Session {
    Messenger *messenger;
    GC_Chat *chats;
    uint32_t num_chats;
    GC_Announce* announce;

    void (*message)(Messenger *m, int, uint32_t, const uint8_t *, uint32_t, void *);
    void *message_userdata;
    void (*private_message)(Messenger *m, int, uint32_t, const uint8_t *, uint32_t, void *);
    void *private_message_userdata;
    void (*op_action)(Messenger *m, int, uint32_t, const uint8_t *, uint32_t, void *);
    void *op_action_userdata;
    void (*nick_change)(Messenger *m, int, uint32_t, const uint8_t *, uint32_t, void *);
    void *nick_change_userdata;
    void (*title_change)(Messenger *m, int, uint32_t, const uint8_t *,  uint32_t, void *);
    void *title_change_userdata;
    void (*peer_join)(Messenger *m, int, uint32_t, void *);
    void *peer_join_userdata;
    void (*self_join)(Messenger *m, int, uint32_t*, uint32_t, void *);
    void *self_join_userdata;
    void (*peer_exit)(Messenger *m, int, uint32_t, const uint8_t *, uint32_t, void *);
    void *peer_exit_userdata;
} GC_Session;

/* Return -1 if fail
 * Return 0 if success
 */
int gc_send_op_action(const GC_Chat *chat, const uint8_t *certificate);

/* Return -1 if fail
 * Return 0 if success
 */
int gc_toggle_ignore(GC_Chat *chat, uint32_t peernumber, uint8_t ignore);

/* Return -1 if fail
 * Return 0 if success
 */
int gc_send_plain_message(const GC_Chat *chat, const uint8_t *message, uint32_t length);

/* Return -1 if fail
 * Return 0 if success
 */
int gc_set_topic(GC_Chat *chat, const uint8_t *topic, uint32_t length);

 /* Return topic length. */
int gc_get_topic(const GC_Chat *chat, uint8_t *topicbuffer);

/* Return -1 if fail
 * Return 0 if success
 */
int gc_set_self_nick(GC_Chat *chat, const uint8_t *nick, uint32_t length);

/* Return -1 on error.
 * Return nick length if success
 */
int gc_get_nick(const GC_Chat *chat, uint32_t peernumber, uint8_t *namebuffer);

/* Return -1 if fail
 * Return 0 if success
 */
int gc_set_self_status(GC_Chat *chat, uint8_t status_type);

/* Return's peernumber's status (GS_INVALID on failure) */
uint8_t gc_get_status(const GC_Chat *chat, uint8_t peernumber);

void gc_callback_message(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                         const uint8_t *, uint32_t, void *), void *userdata);

void gc_callback_private_message(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                                 const uint8_t *, uint32_t, void *), void *userdata);

void gc_callback_op_action(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                           const uint8_t *, uint32_t, void *), void *userdata);

void gc_callback_nick_change(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                             const uint8_t *, uint32_t, void *), void *userdata);

void gc_callback_title_change(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                              const uint8_t *, uint32_t, void *), void *userdata);

void gc_callback_group_self_join(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t*, uint32_t, void *),
                                 void *userdata);

void gc_callback_peer_join(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t, void *),
                           void *userdata);

void gc_callback_peer_exit(Messenger *m, void (*function)(Messenger *m, int groupnumber, uint32_t,
                           const uint8_t *, uint32_t, void *), void *userdata);

/* This is the main loop.
 */
void do_gc(GC_Session* c);

/* Returns a NULL pointer if fail.
 * Make sure that DHT is initialized before calling this
 */
GC_Session* new_groupchats(Messenger* m);

/* Calls gc_group_delete() for every group chat */
void gc_kill_groupchats(GC_Session* c);

/* Adds a new group chat
 *
 * Return groupnumber on success
 * Return -1 on failure
 */
int gc_group_add(GC_Session *c);

/* Creates a group chat and sends an invite request using invite_key
 *
 * Return groupnumber on success.
 * Reutrn -1 on failure.
 */
int gc_group_join(GC_Session *c, const uint8_t *invite_key);

/* Deletes chat from group chat array and cleans up.
 *
 * Return 0 on success.
 * Return -1 on failure.
 */
int gc_group_delete(GC_Session* c, GC_Chat *chat, const uint8_t *partmessage, uint32_t length);

/* Return groupnumber's GC_Chat pointer on success
 * Return NULL on failure
 */
GC_Chat *gc_get_group(const GC_Session* c, int groupnumber);

#endif  /* GROUP_CHATS_H */
