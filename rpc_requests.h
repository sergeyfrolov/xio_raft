//
// Created by sfrolov on 11/16/15.
//

#ifndef XIO_RAFT_RPC_REQUESTS_H
#define XIO_RAFT_RPC_REQUESTS_H

#include <stdint.h>
#include <vector>

using std::vector;

struct AppendEntryRequest {
    uint64_t        term;
    uint16_t        leaderId;
    uint64_t        prevLogIndex;
    uint64_t        prevLogTerm;
    uint16_t        entriesSize;
    uint64_t        leaderCommitIndex;
};
// entries should be in sglist[1]
// char*           entries; // null-terminated

struct AppendEntryReply {
    uint64_t        term;
    bool            success;
};

struct VoteRequestRequest {
    uint64_t        term;
    uint16_t        candidateId;
    uint64_t        lastLogIndex;
    uint64_t        lastLogTerm;
};

struct VoteRequestReply {
    uint64_t        term;
    bool            vote_granted;
};

#endif //XIO_RAFT_RPC_REQUESTS_H