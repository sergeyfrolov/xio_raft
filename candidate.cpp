// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include "candidate.h"
#include "follower.h"
#include "leader.h"
#include "raft_log.h"

void Candidate::becomeFollower() {
    cout << "Going from Candidate to Follower" << endl;
    RaftLog::getInstance().setCurrentState(new Follower);
    delete this;
};

void Candidate::becomeLeader() {
    cout << "Going from Candidate to Leader" << endl;
    RaftLog::getInstance().setCurrentState(new Leader);
    delete this;
};
