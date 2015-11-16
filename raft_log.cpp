// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include "raft_log.h"

RaftLog::RaftLog() {
    term = 0;
    votes = 0;
    changing_state = false;
};

// typical singleton getInstance
RaftLog& RaftLog::getInstance() {
    static RaftLog raftLogSingleton;
    return raftLogSingleton;
}

// State-related fields methods
void RaftLog::setCurrentState(RaftStateInterface *s)
{
    state = s;
}
void RaftLog::becomeFollower() {
    state->becomeFollower();
};
void RaftLog::becomeCandidate() {
    state->becomeCandidate();
};
void RaftLog::becomeLeader() {
    state->becomeLeader();
};

RaftLog::~RaftLog() {
    delete state;
};
