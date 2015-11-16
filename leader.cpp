// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include "leader.h"
#include "follower.h"
#include "raft_log.h"

void Leader::becomeFollower() {
    cout << "Going from Leader to Follower" << endl;
    RaftLog::getInstance().setCurrentState(new Follower);
    delete this;
};

void Leader::becomeCandidate() {
    cerr << "Cannot go from Leader to Candidate!" << endl;
};
