// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_DEFINES_H
#define XIO_RAFT_DEFINES_H

const int RAFT_FOLLOWER     = 0x00000001;
const int RAFT_CANDIDATE    = 0x00000010;
const int RAFT_LEADER       = 0x00000110;

char* HEARTBEAT_RPC            = (char*) "HEARTBEAT_RPC";
char* HEARTBEAT_RPC_REPLY      = (char*) "HEARTBEAT_RPC_REPLY";
char* APPEND_RPC               = (char*) "APPEND_RPC";
char* APPEND_RPC_REPLY         = (char*) "APPEND_RPC_REPLY";
char* REQUEST_VOTE             = (char*) "REQUEST_VOTE";
char* REQUEST_VOTE_REPLY       = (char*) "REQUEST_VOTE_REPLY";
char* CLIENT_APPEND_REQUEST    = (char*) "CLIENT_APPEND_REQUEST";

const int HEARTBEAT_TIMEOUT               = 5;
const int MIN_TIMEOUT_TO_BECOME_CANDIDATE = 10;
const int MAX_TIMEOUT_TO_BECOME_CANDIDATE = 20;
const int MIN_TIMEOUT_TO_RESTART_ELECTION = 10;
const int MAX_TIMEOUT_TO_RESTART_ELECTION = 20;

#endif //XIO_RAFT_DEFINES_H
