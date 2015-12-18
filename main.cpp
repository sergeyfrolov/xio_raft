// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <iostream>
#include <sstream>
#include "raft_log.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Wrong amount of arguments: given " << argc - 1 << ", expected 2." << endl;
        cerr << "Usage: ./dfserver nodes.conf ID" << endl;
        return 1;
    }
    string nodes_conf_file = string(argv[1]);
    string local_id = string(argv[2]);

    RaftLog::getInstance().nodeAddress = RaftLog::parse_config_file(nodes_conf_file);
    RaftLog::getInstance().currentTerm = 1;
    RaftLog::getInstance().id = stoi(local_id);
    RaftLog::getInstance().nodesTotal = RaftLog::getInstance().nodeAddress.size();

    // fill completely RaftLog::getInstance().nodeAddress
    RaftLog::getInstance().initXio();
    RaftLog::getInstance().runXioLoop();

    return 0;
}
