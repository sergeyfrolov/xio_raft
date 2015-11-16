// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_RAFTSTATEINTERFACE_H
#define XIO_RAFT_RAFTSTATEINTERFACE_H

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

class RaftStateInterface {

public:
    // none of those should actually get called, only overriden versions:
    virtual void becomeFollower() {
        cerr << "Already Follower" << endl;
    };
    virtual void becomeCandidate() {
        cerr << "Already Candidate" << endl;
    };
    virtual void becomeLeader() {
        cerr << "Already Leader" << endl;
    };
};


#endif //XIO_RAFT_RAFTSTATEINTERFACE_H
