// Copyright 2015 Sergey Frolov. All rights reserved.
// Use of this source code is governed by a OpenIB.org BSD license that can be
// found in the LICENSE file.

#ifndef XIO_RAFT_XIO_CALLBACKS_H
#define XIO_RAFT_XIO_CALLBACKS_H

#include "raft_state_interface.h"
#include "raft_log.h"
#include "libxio.h"
#include "rpc_requests.h"

namespace xio_raft {
    static int on_request(struct xio_session *session, struct xio_msg *req, int last_in_rxq, void *cb_user_context) {
        std::shared_lock<std::shared_timed_mutex> stateSharedLock;
        stateSharedLock.lock();

        struct server_data *server_data = (struct server_data *) cb_user_context;

        struct xio_iovec_ex *sglist = vmsg_sglist(&req->in);
        //  int			nents = vmsg_sglist_nents(&req->in);
        int state = RaftLog::getInstance().getState();

        std::string header(static_cast<char *>(req->in.header.iov_base), req->in.header.iov_len);

        cout << "#DEBUG Node " << RaftLog::getInstance().id << "receives request with header " << header << endl;

        // append_RPC
        // if RPCterm < currentTerm
        //    reply false
        // if RPCterm > currentTerm
        //    convert to follower
        //    currentTerm = RPCterm
        // # keep order!
        // if local_log[RPCprevLogIndex] not exists or local_log[RPCprevLogIndex] != RPCprevLogTerm
        //   reply false
        // if local_log[RPCindex] already exists
        //    delete it and all the next(resize?)
        //    add new entries
        //    update timer
        //    applyAllUncommitedLogs();
        // if RPCleaderCommit > commitIndex
        //    commitIndex = min(RPCleaderCommit, indexOfLastNewEntry)

        // if leader
        //   if appendEntryResponse
        //     if success
        //       nextIndex[responder_id] =
        //       matchIndex[responder_id] =
        //     if fail
        //       nextIndex[responder_id]--;

        if (strcmp(static_cast<char *>(req->in.header.iov_base), REQUEST_VOTE_REPLY) == 0) {
            if (state == RAFT_CANDIDATE) {
                RaftLog::getInstance().votes++;
            }
            else {
                RaftLog::getInstance().votes = 0;
                cerr << "Non-candidate recieves RequestVoteReply!" << endl;
            }

        }
        else { // sending msg for those
            struct xio_msg *rsp = new xio_msg(*req);
            rsp->request = req;
            rsp->out.sgl_type = XIO_SGL_TYPE_IOV;
            rsp->out.data_iov.max_nents = XIO_IOVLEN;
            rsp->out.data_iov.nents = 1;

            if (strcmp(static_cast<char *>(req->in.header.iov_base), HEARTBEAT_RPC) == 0) {
                AppendEntryRequest *aer = (AppendEntryRequest *) sglist[0].iov_base;
                AppendEntryReply *ae_reply = (AppendEntryReply *) malloc(sizeof(VoteRequestReply) + 1);
                ae_reply->term = RaftLog::getInstance().currentTerm;
                rsp->out.header.iov_base = APPEND_RPC_REPLY;
                if (state == RAFT_FOLLOWER) {
                    if (aer->leaderId == RaftLog::getInstance().votedFor) {
                        ae_reply->success = true;
                    }
                    else if (aer->term < RaftLog::getInstance().currentTerm) {
                        ae_reply->success = false;
                    }
                    else if (aer->term > RaftLog::getInstance().currentTerm) {
                        RaftLog::getInstance().votedFor = aer->leaderId;
                        RaftLog::getInstance().currentTerm = aer->term;
                        ae_reply->success = true;
                    }
                }
                else { // leader or candidate
                    if (aer->term > RaftLog::getInstance().currentTerm) {
                        RaftLog::getInstance().votedFor = aer->leaderId;
                        RaftLog::getInstance().currentTerm = aer->term;
                        ae_reply->success = true;
                        RaftLog::getInstance().becomeFollower();
                    }
                    else
                        ae_reply->success = false;
                }
                *(char *) (ae_reply++) = '\0';
                rsp->out.data_iov.sglist[0].iov_base = ae_reply;
            }
            else if (strcmp(static_cast<char *>(req->in.header.iov_base), REQUEST_VOTE) == 0) {
                VoteRequestRequest *vrr = (VoteRequestRequest *) sglist[0].iov_base;
                VoteRequestReply *vr_reply = (VoteRequestReply *) malloc(sizeof(VoteRequestReply) + 1);
                vr_reply->term = RaftLog::getInstance().currentTerm;
                if ((RaftLog::getInstance().votedFor == 0) || (RaftLog::getInstance().votedFor == vrr->candidateId)) {
                    vr_reply->vote_granted = true;
                }
                else if ((vrr->lastLogTerm > RaftLog::getInstance().log.back().term) &&
                         (vrr->lastLogIndex >= RaftLog::getInstance().log.size() - 1)) {
                    vr_reply->vote_granted = true;
                }
                else {
                    vr_reply->vote_granted = false;
                }
                *(char *) (vr_reply++) = '\0';
                rsp->out.data_iov.sglist[0].iov_base = vr_reply;
            }
            rsp->out.header.iov_len = strlen((const char *) rsp->out.header.iov_base) + 1;
            rsp->out.data_iov.sglist[0].iov_len = strlen((const char *) rsp->out.data_iov.sglist[0].iov_base) + 1;
            xio_send_response(rsp);
        }

        // if CLIENT_APPEND_REQUEST
        //     append

        stateSharedLock.unlock();
        return 0;
    }

    // TODO: on response finish: free mem
   
    static int on_session_event(struct xio_session *session, struct xio_session_event_data *event_data,
                                void *cb_user_context) {
        struct session_data *session_data = (struct session_data *) cb_user_context;

        printf("session event: %s. session:%p, connection:%p, reason: %s\n",
               xio_session_event_str(event_data->event),
               session, event_data->conn,
               xio_strerror(event_data->reason));

        switch (event_data->event) {
            case XIO_SESSION_NEW_CONNECTION_EVENT:
                session_data->conn = event_data->conn;
                break;
            case XIO_SESSION_CONNECTION_TEARDOWN_EVENT:
                fprintf(stderr, "Tear down the connection...\n");
                xio_connection_destroy(event_data->conn);
                session_data->conn = NULL;
                break;
            case XIO_SESSION_TEARDOWN_EVENT:
                fprintf(stderr, "Destroy the session...\n");
                xio_session_destroy(session);
                // xio_context_stop_loop(session_data->ctx);  /* exit */
                break;
            default:
                break;
        };

        return 0;
    }

    static int on_new_session(struct xio_session *session, struct xio_new_session_req *req, void *cb_user_context) {
        struct session_data *session_data = (struct session_data *) cb_user_context;

        /* automatically accept the request */
        fprintf(stderr, "new session event. session:%p\n", session);

        if (!session_data->conn)
            xio_accept(session, NULL, 0, NULL, 0);
        else
            xio_reject(session, (enum xio_status) EISCONN, NULL, 0);

        fprintf(stderr, "Leaving on_new_session\n");
        return 0;
    }


}
#endif //XIO_RAFT_XIO_CALLBACKS_H
