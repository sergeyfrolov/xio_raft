// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <unistd.h>
#include "candidate.h"
#include "follower.h"
#include "leader.h"
#include "raft_log.h"

Candidate::Candidate() {
    RaftLog::getInstance().stateUniqueLock.lock();
    RaftLog::getInstance().votes = 1;
    RaftLog::getInstance().currentTerm++;

    vote_request.candidateId = RaftLog::getInstance().id;
    vote_request.lastLogIndex = RaftLog::getInstance().log.size();
    vote_request.lastLogTerm = RaftLog::getInstance().log[RaftLog::getInstance().log.size() - 1].term;
    vote_request.term = RaftLog::getInstance().currentTerm;
    sendRequestsForVote();
    pollT = std::move(std::thread(&Candidate::pollYourself, this));
    currentState = RAFT_CANDIDATE;
    RaftLog::getInstance().stateUniqueLock.unlock();
}

Candidate::~Candidate() {
    RaftLog::getInstance().stateUniqueLock.lock();
    to_die = true;
    if (pollT.joinable()) pollT.join();
    RaftLog::getInstance().stateUniqueLock.unlock();
}

void Candidate::sendRequestsForVote() {
    xio_vote_request.in.sgl_type = XIO_SGL_TYPE_IOV;
    xio_vote_request.in.data_iov.max_nents = XIO_IOVLEN;
    xio_vote_request.in.data_iov.nents = 0;

    xio_vote_request.out.header.iov_base = REQUEST_VOTE;
    xio_vote_request.out.header.iov_len = 13;

    xio_vote_request.out.sgl_type = XIO_SGL_TYPE_IOV;
    xio_vote_request.out.data_iov.max_nents = XIO_IOVLEN;
    xio_vote_request.out.data_iov.sglist[0].iov_base = reinterpret_cast<void*>(&vote_request);
    xio_vote_request.out.data_iov.sglist[0].iov_len = sizeof(vote_request) + 1;
    xio_vote_request.out.data_iov.nents = 1;
    // maybe use set<ids> and its size, so noone could possible vote twice?
    election_start = time(NULL);
    for (int i = 1; i < RaftLog::getInstance().nodesTotal; i++) {
        if (i != RaftLog::getInstance().id)
            xio_send_msg(RaftLog::getInstance().xio_session_data[i].conn, &xio_vote_request);
    }
}

void Candidate::pollYourself() {
    while (!to_die) {
        if (RaftLog::getInstance().votes > (RaftLog::getInstance().nodesTotal / 2 + 1))
            return becomeLeader();

        time_t timeout_to_restart_election = election_start + MIN_TIMEOUT_TO_RESTART_ELECTION +
                rand() % (MAX_TIMEOUT_TO_RESTART_ELECTION - MIN_TIMEOUT_TO_RESTART_ELECTION);
        double time_passed_since_update = difftime(time(NULL), election_start);
        if (time_passed_since_update > timeout_to_restart_election) {
            RaftLog::getInstance().votes = 1;
            sendRequestsForVote(); // TODO: theoretically, the same people could vote twice
        }
        sleep(0);
    }
}

void Candidate::becomeFollower() {
    RaftLog::getInstance().stateUniqueLock.lock();
    cout << "Going from Candidate to Follower" << endl;
    delete this;
    RaftLog::getInstance().setCurrentState(new Follower);
    RaftLog::getInstance().stateUniqueLock.unlock();
};

void Candidate::becomeLeader() {
    RaftLog::getInstance().stateUniqueLock.lock();
    cout << "Going from Candidate to Leader" << endl;
    delete this;
    RaftLog::getInstance().setCurrentState(new Leader);
    RaftLog::getInstance().stateUniqueLock.unlock();
};
