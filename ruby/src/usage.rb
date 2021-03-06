# frozen_string_literal: true

#
# jomiel-client-demos
#
# Copyright
#  2019-2021 Toni Gündoğdu
#
#
# SPDX-License-Identifier: Apache-2.0
#
module Usage
  USAGE = <<~HELP
    Usage:
      demo [-hDVjcq] [-r <addr>] [-t <time>] [URI ...]

    Options:
      -h --help                     Print this help and exit
      -D --print-config             Print configuration values and exit
      -V --version-zmq              Display ZeroMQ version and exit
      -j --output-json              Print dumped messages in JSON
      -c --compact-json             Use more compact representation of JSON
      -r --router-endpoint <addr>   Specify the router endpoint address
                                      [default: tcp://localhost:5514]
      -t --connect-timeout <time>   Specify maximum time in seconds for
                                      the connection allowed to take
                                      [default: 30]
      -q --be-terse                 Be brief and to the point; dump
                                      interesting details only
  HELP
end
