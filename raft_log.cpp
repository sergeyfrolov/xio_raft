// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <thread>
#include "raft_log.h"
#include "follower.h"
#include "xio_callbacks.h"
#include <sstream>
#include <fstream>

using std::ifstream;
using std::istringstream;


RaftLog::RaftLog() {
    currentTerm = 0;
    votes = 0;
    state = new Follower();
};

void RaftLog::runXioLoop() {
    while(1) {
        for (int i = 1; i < nodesTotal; i++) {
            if (i != id)
                xio_context_run_loop(xio_session_data[i].ctx, 10);
            else
                xio_context_run_loop(xio_session_data[i].ctx, 100);
        }
    }
}

// typical singleton getInstance
RaftLog& RaftLog::getInstance() {
    static RaftLog raftLogSingleton;
    return raftLogSingleton;
}

// State-related fields methods
void RaftLog::setCurrentState(RaftStateInterface *s) {
    state = s;
}
void RaftLog::becomeFollower() {
    state->becomeFollower();
};
void RaftLog::becomeCandidate() {
    state->becomeCandidate();
};
void RaftLog::becomeLeader() {
    state->becomeLeader();
};

void RaftLog::applyAllUncommitedLogs() {
    while (commitIndex > lastApplied) {
        applyToStateMachine(lastApplied);
        lastApplied++;
    }
}

void RaftLog::initXio() {
    memset(&xio_session_ops, 0, sizeof(xio_session_ops));
    xio_session_ops.on_session_event = xio_raft::on_session_event;
    xio_session_ops.on_new_session = xio_raft::on_new_session;
    xio_session_ops.on_msg_send_complete = NULL;
    xio_session_ops.on_msg = xio_raft::on_request;
    xio_session_ops.on_msg_error = NULL;

    xio_cparams         = new xio_connection_params[nodesTotal + 1];
    xio_params          = new xio_session_params[nodesTotal + 1];
    xio_session_data    = new session_data[nodesTotal + 1];
    xio_sessions        = new xio_session*[nodesTotal + 1];

    xio_init();

    /* Logging ... */
    if (0) {
        int level = XIO_LOG_LEVEL_DEBUG;
        xio_set_opt(NULL, XIO_OPTLEVEL_ACCELIO, XIO_OPTNAME_LOG_LEVEL, &level, sizeof(level));
    }
    memset(xio_params, 0, sizeof(xio_params));
    memset(xio_cparams, 0, sizeof(xio_cparams));
    memset(xio_session_data, 0, sizeof(xio_session_data));

    /* create thread context for the client */
    cout << "Initializing " << id << " out of " << nodesTotal << endl;
    xio_session_data[id].ctx = xio_context_create(NULL, 0, -1);
    xio_server = xio_bind(xio_session_data[id].ctx, &xio_session_ops,
                          nodeAddress[id].c_str(), NULL, 0, &xio_session_data[id]);
    if (xio_server) {
        std::cout << "Bound to " << nodeAddress[id] << std::endl;
    }
    else {
        std::cerr << "Error! Could not bind to " << nodeAddress[id] << std::endl;
    }

    sleep(5);

    for(int i = 1; i < nodesTotal; i++) {
        if (i != id) {
            xio_session_data[i].ctx = xio_context_create(NULL, 0, -1);

            xio_params[i].type = XIO_SESSION_CLIENT;
            xio_params[i].ses_ops = &xio_session_ops;
            xio_params[i].user_context = &xio_session_data[i];
            xio_params[i].uri = nodeAddress[i].c_str();

            xio_sessions[i] = xio_session_create(&xio_params[i]);

            xio_cparams[i].session = xio_sessions[i];
            xio_cparams[i].ctx = xio_session_data[i].ctx;
            xio_cparams[i].conn_user_context = &xio_session_data[i];

            /* connect the session  */
            xio_session_data[i].conn = xio_connect(&xio_cparams[i]);
        }
    }

    //std::thread(&RaftLog::runXioLoop, this).detach();
}

void RaftLog::applyToStateMachine(uint64_t indexToApply) {
    // Override me
    cout << "Applying \"" << log[indexToApply].entry << "\" to State Machine. " <<
            "EntryTerm = " << log[indexToApply].term << endl;
    return;
}

int RaftLog::getState() {
    return state->currentState;
}

RaftLog::~RaftLog() {
    delete state;
};

std::map<int, string> RaftLog::parse_config_file(string filename) {
    ifstream ws;
    ws.open(filename);
    if (!ws.good()) {
        throw std::runtime_error("Could not open " + filename);
    }
    char *line_buf = new char[1024];
    std::map<int, string> parsed_dict;

    while (!ws.eof()) {
        ws.getline(line_buf, 1024);
        if (line_buf[0]) {
            istringstream iss(line_buf);
            string var_name, var_value;
            std::getline(iss, var_name, ' ');
            std::getline(iss, var_value, '#');
            if ((var_name[0] != '#') && (!var_value.empty()))
                parsed_dict[stoi(var_name)] = var_value;
        }
    }
    cout << "Parsed nodes from " << filename << endl;
        for (auto const &it : parsed_dict)
            cout << it.first << " - " << it.second << endl;
        cout << endl;
    return parsed_dict;
}
