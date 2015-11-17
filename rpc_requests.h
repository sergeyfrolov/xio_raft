//
// Created by sfrolov on 11/16/15.
//

#ifndef XIO_RAFT_RPC_REQUESTS_H
#define XIO_RAFT_RPC_REQUESTS_H

#include <stdint.h>
#include <array>

using std::array;

struct AppendEntryRequest {
    uint64_t        term;
    uint8_t         leaderId;
    uint64_t        prevLogIndex;
    uint64_t        prevLogTerm;
    array<char *>   entries;
    uint64_t        leaderCommitIndex;
};

struct AppendEntryReply {
    uint64_t        term;
    bool            success;
};

struct VoteRequestRequest {
    uint64_t        term;
    uint8_t         candidateId;
    uint64_t        lastLogIndex;
    uint64_t        lastLogTerm;
};

struct VoteRequestReply {
    uint64_t        term;
    bool            vote_granted;
};

#endif //XIO_RAFT_RPC_REQUESTS_H