// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_RAFT_LOG_H
#define XIO_RAFT_RAFT_LOG_H

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
    uint64_t term, votes;
    bool changing_state;

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
