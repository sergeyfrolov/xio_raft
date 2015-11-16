// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_LEADER_H
#define XIO_RAFT_LEADER_H

#include "raft_state_interface.h"

class Leader : public RaftStateInterface {
    void becomeFollower();
    void becomeCandidate();
};

#endif //XIO_RAFT_LEADER_H
