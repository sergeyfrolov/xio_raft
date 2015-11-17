// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_FOLLOWER_H
#define XIO_RAFT_FOLLOWER_H

#include "raft_state_interface.h"

class Follower : public RaftStateInterface {

    void becomeCandidate();
    void becomeLeader();
};

#endif //XIO_RAFT_FOLLOWER_H