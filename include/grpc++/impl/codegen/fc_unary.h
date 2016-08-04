/*
 *
 * Copyright 2016, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRPCXX_IMPL_CODEGEN_FC_UNARY_H
#define GRPCXX_IMPL_CODEGEN_FC_UNARY_H

#include <grpc++/impl/codegen/call.h>
#include <grpc++/impl/codegen/completion_queue.h>
#include <grpc++/impl/codegen/core_codegen_interface.h>
#include <grpc++/impl/codegen/server_context.h>
#include <grpc++/impl/codegen/sync_stream.h>

namespace grpc {
/// A class to represent a flow-controlled unary call. This is something
/// of a hybrid between conventional unary and streaming. This is invoked
/// through a unary call on the client side, but the server responds to it
/// as though it were a single-ping-pong streaming call. The server can use
/// the \a NextMessageSize method to determine an upper-bound on the size of
/// the message.
/// A key difference relative to streaming: an FCUnary must have exactly 1 Read
/// and exactly 1 Write, in that order, to function correctly.
/// Otherwise, the RPC is in error.
template <class RequestType, class ResponseType>
class FCUnary GRPC_FINAL
    : public ServerReaderWriterInterface<ResponseType, RequestType> {
 public:
  FCUnary(Call* call, ServerContext* ctx)
      : ServerReaderWriterInterface<ResponseType, RequestType>(call, ctx),
        read_done_(false),
        write_done_(false) {}

  ~FCUnary() {}

  bool Read(RequestType* request) GRPC_OVERRIDE {
    if (read_done_) {
      return false;
    }
    read_done_ = true;
    return ServerReaderWriterInterface<ResponseType, RequestType>::Read(
        request);
  }

  using WriterInterface<ResponseType>::Write;
  bool Write(const ResponseType& response,
             const WriteOptions& options) GRPC_OVERRIDE {
    if (write_done_ || !read_done_) {
      return false;
    }
    write_done_ = true;
    return ServerReaderWriterInterface<ResponseType, RequestType>::Write(
        response, options);
  }

 private:
  bool read_done_;
  bool write_done_;
};
}  // namespace grpc

#endif  // GRPCXX_IMPL_CODEGEN_FC_UNARY_H
