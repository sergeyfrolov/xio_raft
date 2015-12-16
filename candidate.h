// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_CANDIDATE_H
#define XIO_RAFT_CANDIDATE_H

#include <xio_user.h>
#include <thread>
#include "raft_state_interface.h"
#include "rpc_requests.h"

class Candidate : public RaftStateInterface {
    time_t              election_start;
    VoteRequestRequest  vote_request;
    xio_msg             xio_vote_request;
    bool                to_die = false;
    std::thread         pollT;
public:
    Candidate();
    ~Candidate();
    void sendRequestsForVote();
    void pollYourself();
    void becomeFollower();
    void becomeLeader();
};

#endif //XIO_RAFT_CANDIDATE_H
