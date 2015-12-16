// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_RAFT_LOG_H
#define XIO_RAFT_RAFT_LOG_H

#include "libxio.h"

#include <vector>
#include <string>
#include <atomic>
#include <shared_mutex>
#include <forward_list>
#include <map>
#include <xio_base.h>

#include "defines.h"
#include "raft_state_interface.h"

using std::string;

class RaftStateInterface;

struct session_data;
struct logEntry;
/* Due to accelio callbacks signatures, being pointers to non-class member functions,
 * we have to use static functions and global states.
 * Luckily, raft object is somewhat fine being a singleton.
 */
class RaftLog {
protected:
    RaftStateInterface* state;
    mutable std::shared_timed_mutex stateMutex; // make it shared_mutex when c++17 comes out -_-

    RaftLog();
    ~RaftLog();
public:
    std::map<int, std::string>    nodeAddress;

    uint16_t                      id;
    uint16_t                      nodesTotal;
    // updated on stable storage
    uint64_t                      currentTerm = 0;
    uint16_t                      votedFor = 0; // no one should have ID = 0
    std::vector<logEntry>         log;

    // not updated on stable storage
    uint64_t                      commitIndex = 0; // last committed
    uint64_t                      lastApplied = 0; // last applied to state machine
    uint64_t                      lastCommitAckdToClient = 0;

    // for candidate
    std::atomic_uint_fast8_t      votes;

    // for leaders
    std::vector<uint64_t>         nextIndex;  // next to send
    std::vector<uint64_t>         matchIndex; // known to be replicated

    // locks
    std::shared_lock<std::shared_timed_mutex> stateSharedLock;
    std::unique_lock<std::shared_timed_mutex> stateUniqueLock;

    // xio stuff
    struct xio_session_ops          xio_session_ops;
    struct xio_session**            xio_sessions;
    struct xio_server*              xio_server;
    struct server_data*             xio_server_data;
    struct session_data*            xio_session_data;
    struct xio_session_params*      xio_params;
    struct xio_connection_params*   xio_cparams;

    // func
    static RaftLog& getInstance();

    // State-related methods
    void setCurrentState(RaftStateInterface *s);
    void becomeFollower();
    void becomeCandidate();
    void becomeLeader();
    int  getState();

    // StateMachine functions
    void applyAllUncommitedLogs();
    virtual void applyToStateMachine(uint64_t indexToApply); // override, if state machine needs action

    // TODO:
    // void saveLogToDrive();
    void initXio();
    void runXioLoop();

    static std::map<int, string> parse_config_file(string filename);
private:
    // deleting copy and assignment constructors
    RaftLog(RaftLog const&)        = delete;
    void operator=(RaftLog const&) = delete;
};

struct logEntry {
    uint64_t term;
    std::string entry;
};

struct session_data {
    struct xio_context          *ctx;
    struct xio_connection       *conn;
};



#endif //XIO_RAFT_RAFT_LOG_H
