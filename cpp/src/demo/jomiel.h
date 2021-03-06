/*
 * -*- coding: utf-8 -*-
 *
 * jomiel-client-demos
 *
 * Copyright
 *  2019-2021 Toni Gündoğdu
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <docopt/docopt.h>
#include <google/protobuf/message.h>

#include <memory>
#include <string>
#include <zmq.hpp>

#include "jomiel/protobuf/v1beta1/message.pb.h"

namespace jp = jomiel::protobuf::v1beta1;
namespace gp = google::protobuf;

namespace jomiel {

using opts_t = std::map<std::string, docopt::value>;
using zitems_t = std::vector<zmq::pollitem_t>;

struct jomiel {
  explicit jomiel(opts_t const &);
  void inquire() const;

private:
  void connect() const;

  void print_message(std::string const &, gp::Message const &) const;
  void print_status(std::string const &) const;

  void dump_terse_response(jp::MediaResponse const &) const;
  void dump_response(jp::Response const &) const;

  void send_inquiry(std::string const &) const;
  void receive_response() const;

  void to_json(gp::Message const &, std::string &) const;
  void cleanup() const;

private:
  void compat_zmq_set_options() const;
  void compat_zmq_read(zmq::message_t &) const;
  void compat_zmq_send(std::string const &) const;
  void compat_zmq_pollitems(zitems_t &) const;
  void compat_zmq_parse(jp::Response &, zmq::message_t const &) const;

private:
  using zctx_t = std::unique_ptr<zmq::context_t>;
  using zsck_t = std::unique_ptr<zmq::socket_t>;
  struct {
    zctx_t ctx;
    zsck_t sck;
  } zmq;
  opts_t opts;
};

} // namespace jomiel

// vim: set ts=2 sw=2 tw=72 expandtab:
