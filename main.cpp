// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <iostream>
#include "libxio.h"
#include "raft_log.h"

namespace xio_raft {
#define FOLLOWER    0x00000001
#define CANDIDATE   0x00000010
#define LEADER      0x00000110

}

int main() {
    xio_init();

    RaftLog::getInstance().term = 1;

    return 0;
}