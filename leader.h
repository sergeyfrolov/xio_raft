// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_LEADER_H
#define XIO_RAFT_LEADER_H

#include <mutex>
#include <list>
#include "raft_state_interface.h"
#include "rpc_requests.h"
#include <map>
#include <thread>
#include <xio_user.h>

struct ChunkIndex {
    uint64_t startIndex;
    uint64_t length;
    ChunkIndex(uint64_t st, uint64_t l): startIndex(st), length(l) {}
//        bool operator<(const ChunkIndex& lhs, const ChunkIndex& rhs) {
//           return (lhs.startIndex < rhs.startIndex);
//      }
};

inline bool operator<(ChunkIndex const& left, ChunkIndex const& right) {
    if (left.startIndex < right.startIndex) { return true; }
    if (left.startIndex > right.startIndex) { return false; }
    return left.length < right.length;
}

struct RPC {
    char* header;
    char* RPC;
    char* entries;
};

class Leader : public RaftStateInterface {
    time_t        last_contact;
    std::thread   pollT;
    bool          to_die = false;
public:
    AppendEntryRequest heartbeat;
    xio_msg            heartbeat_request;

    std::map<ChunkIndex, RPC> RPCrequests;
    Leader();
    ~Leader();

    void sendAppendEntryRPCs(); // send appendEntriesRPC to everyone for entries between nextIndex and log.size()
    void sendResponseToClient();
    void becomeFollower();
    void becomeCandidate();
private:
    uint64_t findLastCommited(uint64_t first, uint64_t last);
    bool checkIfCommited(uint64_t index);

    void pollForHeartbeat();
    void sendHeartbeat();
};

#endif //XIO_RAFT_LEADER_H
