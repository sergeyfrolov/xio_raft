// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <unistd.h>
#include "follower.h"
#include "candidate.h"
#include "raft_log.h"
#include <thread>

Follower::Follower() {
    // do not need to keep track of this thread, as it finishes execution in only case of destruction:
    //    becoming candidate after timeout
    std::thread(&Follower::pollLastUpdFromLeader, this).detach();
    currentState = RAFT_FOLLOWER;
}

void Follower::becomeCandidate(){
    RaftLog::getInstance().stateUniqueLock.lock();
    cout << "Going from Follower to Candidate" << endl;
    delete this;
    RaftLog::getInstance().setCurrentState(new Candidate);
    RaftLog::getInstance().stateUniqueLock.unlock();
};

void Follower::pollLastUpdFromLeader(){
    while(1) {
        time_t timeout_to_become_candidate = leaderTimeout + MIN_TIMEOUT_TO_BECOME_CANDIDATE +
                                             rand() %
                                             (MAX_TIMEOUT_TO_BECOME_CANDIDATE - MIN_TIMEOUT_TO_BECOME_CANDIDATE);
        double time_passed_since_update = difftime(time(NULL), lastUpdFromLeader);
        if (time_passed_since_update > timeout_to_become_candidate) {
            return becomeCandidate();
        }
        sleep(timeout_to_become_candidate - time_passed_since_update);
    }
}

void Follower::becomeLeader() {
    cerr << "Cannot go from Follower to Leader!" << endl;
};