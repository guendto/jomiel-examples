/*
 * -*- coding: utf-8 -*-
 *
 * jomiel-examples
 *
 * Copyright
 *  2019-2021 Toni Gündoğdu
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "demo/jomiel.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

#include <cstdio>
#include <sstream>
#include <string>

#include "jomiel/protobuf/v1beta1/status.pb.h"

namespace jomiel {

jomiel::jomiel(opts_t const &opts) : opts(opts) {
  zmq.endpoint = opts.at("--router-endpoint").asString();
  zmq.timeout = opts.at("--connect-timeout").asLong();

  zmq.ctx = std::make_unique<zmq::context_t>(1);
  zmq.sck = std::make_unique<zmq::socket_t>(*zmq.ctx, ZMQ_REQ);
#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 7, 0)
  zmq.sck->set(zmq::sockopt::linger, 0);
#else
  int n = 0;
  zmq.sck->setsockopt(ZMQ_LINGER, &n, sizeof(n));
#endif
}

void jomiel::connect() const {
  std::ostringstream stream;

  stream << "<connect> " << zmq.endpoint << " (timeout=" << zmq.timeout
         << ")\n";

  print_status(stream.str());
  zmq.sck->connect(zmq.endpoint);
}

void jomiel::inquire(std::string const &uri) const {
  send(uri);
  recv();
}

void jomiel::send(std::string const &uri) const {
  jp::Inquiry inquiry;
  inquiry.mutable_media()->set_input_uri(uri);

  if (!opts.at("--be-terse").asBool()) {
    print_message("<send>", inquiry);
  }

  std::string result;
  inquiry.SerializeToString(&result);

#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 4, 0)
  zmq.sck->send(zmq::buffer(result));
#else
  zmq.sck->send(result.c_str(), result.size());
#endif
}

void jomiel::recv() const {
#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1)
  zmq::pollitem_t items[] = {{*zmq.sck, 0, ZMQ_POLLIN, 0}};
#else
  zmq::pollitem_t const items[] = {
      {static_cast<void *>(*zmq.sck), 0, ZMQ_POLLIN}};
#endif
  zmq::message_t msg;

  if (zmq::poll(&items[0], 1, zmq.timeout * 1000))
#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1)
    zmq.sck->recv(msg);
#else
    zmq.sck->recv(&msg);
#endif
  else
    throw std::runtime_error("connection timed out");

  jp::Response response;
#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 6, 0)
  response.ParseFromString(msg.to_string());
#else
  const std::string str(static_cast<char *>(msg.data()), msg.size());
  response.ParseFromString(str);
#endif

  dump_response(response);
}

void jomiel::cleanup() const {
  /*
   * "Also notice the call to ShutdownProtobufLibrary() at the end of
   * the program. All this does is delete any global objects that were
   * allocated by the Protocol Buffer library. This is unnecessary for
   * most programs, since the process is just going to exit anyway and
   * the OS will take care of reclaiming all of its memory. However,
   * if you use a memory leak checker that requires that every last
   * object be freed, or if you are writing a library which may be
   * loaded and unloaded multiple times by a single process, then you
   * may want to force Protocol Buffers to clean up everything."
   *  -- https://developers.google.com/protocol-buffers/docs/cpptutorial
   */
  gp::ShutdownProtobufLibrary();
}

void jomiel::print_status(std::string const &status) const {
  if (!opts.at("--be-terse").asBool()) {
    std::clog << "status: " << status;
  }
}

void jomiel::print_message(std::string const &status,
                           gp::Message const &msg) const {
  std::string result;

  if (opts.at("--output-json").asBool()) {
    gp::util::JsonPrintOptions opts;
    opts.add_whitespace = true;

    MessageToJsonString(msg, &result, opts);
  } else {
    gp::TextFormat::PrintToString(msg, &result);
  }

  print_status(status);
  std::cout << "\n" << result;
}

void jomiel::dump_terse_response(
    jp::MediaResponse const &media_response) const {
  std::cout << "---\ntitle: " << media_response.title() << "\n";
  std::cout << "quality:\n";

  for (auto const &stream : media_response.stream()) {
    auto const &quality = stream.quality();
    std::ostringstream format;
    format << "  profile: " << quality.profile() << "\n"
           << "    width: " << quality.width() << "\n"
           << "    height: " << quality.height() << "\n";
    std::cout << format.str();
  }
}

void jomiel::dump_response(jp::Response const &response) const {
  auto const &response_status = response.status();
  auto const &media_response = response.media();

  if (response_status.code() == jp::STATUS_CODE_OK) {
    if (opts.at("--be-terse").asBool()) {
      dump_terse_response(media_response);
    } else {
      print_message("<recv>", media_response);
    }
  } else {
    print_message("<recv>", response);
  }
}

} // namespace jomiel

// vim: set ts=2 sw=2 tw=72 expandtab:
