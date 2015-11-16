// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include "follower.h"
#include "candidate.h"
#include "raft_log.h"

void Follower::becomeCandidate(){
    cout << "Going from Follower to Candidate" << endl;
    RaftLog::getInstance().setCurrentState(new Candidate);
    delete this;
};

void Follower::becomeLeader() {
    cerr << "Cannot go from Follower to Leader!" << endl;
};