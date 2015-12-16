// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <map>
#include <unistd.h>
#include "leader.h"
#include "follower.h"
#include "rpc_requests.h"
#include "raft_log.h"

Leader::Leader(){
    for(int i = 1; i <= RaftLog::getInstance().nodesTotal ; i++)
        RaftLog::getInstance().nextIndex[i] = RaftLog::getInstance().log.size();
    sendAppendEntryRPCs();
    currentState = RAFT_LEADER;
    pollT = std::move(std::thread(&Leader::pollForHeartbeat, this));
}

Leader::~Leader(){
    RaftLog::getInstance().stateUniqueLock.lock();
    to_die = true;
    if (pollT.joinable()) pollT.join();
    RaftLog::getInstance().stateUniqueLock.unlock();
}

void Leader::sendAppendEntryRPCs() {
    uint64_t currSize = RaftLog::getInstance().log.size();
    for (int i = 1; i <= RaftLog::getInstance().nodesTotal; i++) {
        if ( i != RaftLog::getInstance().id) {
            uint64_t indexToSendStart = RaftLog::getInstance().nextIndex[i];
            ChunkIndex currentChunkIndex(indexToSendStart, currSize - indexToSendStart);
            std::map<ChunkIndex, RPC>::iterator it = RPCrequests.find(currentChunkIndex);
            RPC req;
            if(it != RPCrequests.end())
            {
                // element found;
                req = it->second;
            }
            else {
                // TODO: construct request
                // put it in map
                // req = contructed_req
            }
            // TODO: send request req
        }
        else
            RaftLog::getInstance().nextIndex[i]++;
    }

    // check for committedIndexes by binary search and answer to client
    RaftLog::getInstance().commitIndex = findLastCommited(RaftLog::getInstance().commitIndex, currSize);
    sendResponseToClient();
}

uint64_t Leader::findLastCommited(uint64_t first, uint64_t last) {
    uint64_t middle = (first + last) / 2;
    while (first != middle) {
        if (checkIfCommited(middle))
            first = middle;
        else
            last = middle;
        middle = (first + last) / 2;
    }
    if (checkIfCommited(middle + 1))
        return middle + 1;
    else
        return middle;
}

bool Leader::checkIfCommited(uint64_t index) {
    RaftLog* raftLog = &RaftLog::getInstance();
    if (raftLog->currentTerm == raftLog->log[index].term) {
        uint64_t commited_count = 0;
        for (int i = 1; i < raftLog->nodesTotal; i++) {
            if (raftLog->matchIndex[i] >= index)
                commited_count++;
        }
        if (commited_count >= (raftLog->nodesTotal / 2 + 1))
            return true;
    }
    return false;
}

void Leader::sendResponseToClient() {
    while (RaftLog::getInstance().commitIndex > RaftLog::getInstance().lastCommitAckdToClient) {
        // TODO: ACK commit
        RaftLog::getInstance().lastCommitAckdToClient++;
    }
}

void Leader::pollForHeartbeat() {
    heartbeat.entriesSize = 0;
    heartbeat.leaderId = RaftLog::getInstance().id;
    while (!to_die) {
        double time_passed_since_contact = difftime(time(NULL), last_contact);
        if (time_passed_since_contact > HEARTBEAT_TIMEOUT) {
            sendHeartbeat();
        }
        sleep(0);
    }
}

void Leader::sendHeartbeat() {
    heartbeat.term = RaftLog::getInstance().currentTerm;
    heartbeat.leaderCommitIndex = RaftLog::getInstance().commitIndex;
    heartbeat.prevLogIndex = RaftLog::getInstance().log.size() - 1;
    heartbeat.prevLogTerm = RaftLog::getInstance().log.back().term;

    heartbeat_request.in.sgl_type = XIO_SGL_TYPE_IOV;
    heartbeat_request.in.data_iov.max_nents = XIO_IOVLEN;
    heartbeat_request.in.data_iov.nents = 0;

    heartbeat_request.out.header.iov_base = REQUEST_VOTE;
    heartbeat_request.out.header.iov_len = 13;

    heartbeat_request.out.sgl_type = XIO_SGL_TYPE_IOV;
    heartbeat_request.out.data_iov.max_nents = XIO_IOVLEN;
    heartbeat_request.out.data_iov.sglist[0].iov_base = reinterpret_cast<void*>(&heartbeat);
    heartbeat_request.out.data_iov.sglist[0].iov_len = sizeof(heartbeat) + 1;
    heartbeat_request.out.data_iov.nents = 1;

    for (int i = 1; i < RaftLog::getInstance().nodesTotal; i++) {
        cout << "#DEBUG Leader (" << RaftLog::getInstance().id << ") is sending out heartbeats" << endl;
        if (i != RaftLog::getInstance().id)
            xio_send_msg(RaftLog::getInstance().xio_session_data[i].conn, &heartbeat_request);
    }
}

void Leader::becomeFollower() {
    RaftLog::getInstance().stateUniqueLock.lock();
    cout << "Going from Leader to Follower" << endl;
    delete this;
    RaftLog::getInstance().setCurrentState(new Follower);
    RaftLog::getInstance().stateUniqueLock.unlock();
};

void Leader::becomeCandidate() {
    cerr << "Cannot go from Leader to Candidate!" << endl;
};

