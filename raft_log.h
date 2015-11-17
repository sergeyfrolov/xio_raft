// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_RAFT_LOG_H
#define XIO_RAFT_RAFT_LOG_H

#include <array>
#include <string>
#include "raft_state_interface.h"

/* Due to accelio callbacks signatures, being pointers to non-class member functions,
 * we have to use static functions and global states.
 * Luckily, raft object is somewhat fine being a singleton.
 */
class RaftLog {
protected:
    RaftStateInterface* state;

    RaftLog();
    ~RaftLog();
public:
    // updated on stable storage
    uint64_t    term = 0;
    uint8_t     votedFor = NULL;
    bool        changing_state = false;

    // not updated on stable storage
    uint64_t    commitIndex = 0; // last committed
    uint64_t    lastApplied = 0; // last applied to state machine

    // for candidates
    uint8_t     votes = 0;

    // fot leaders
    std::array<uint8_t>    nextIndex;  // next to send
    std::array<uint8_t>    matchIndex; // known to be replicated

    std::array<std::string> log;

    static RaftLog& getInstance();

    // State-related methods
    void setCurrentState(RaftStateInterface *s);
    void becomeFollower();
    void becomeCandidate();
    void becomeLeader();

private:
    // deleting copy and assignment constructors
    RaftLog(RaftLog const&)        = delete;
    void operator=(RaftLog const&) = delete;
};

#endif //XIO_RAFT_RAFT_LOG_H
