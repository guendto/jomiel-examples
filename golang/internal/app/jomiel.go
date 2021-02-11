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

package app

import (
	"encoding/json"
	"fmt"
	"log"

	proto "github.com/golang/protobuf/proto"
	// czmq "gopkg.in/zeromq/goczmq.v4"  // See README.md for Notes
	czmq "github.com/zeromq/goczmq"

	msgs "internal/gen/messages"
)

type jomiel struct {
	sock *czmq.Sock
	opts *options
}

func newJomiel(opts *options) *jomiel {
	sck := czmq.NewSock(czmq.Req)
	sck.SetOption(czmq.SockSetLinger(0))
	return &jomiel{
		opts: opts,
		sock: sck,
	}
}

func (j *jomiel) Destroy() {
	defer j.sock.Destroy()
}

func (j *jomiel) connect() {
	re := j.opts.RouterEndpoint
	to := j.opts.ConnectTimeout

	status := fmt.Sprintf("<connect> %s (timeout=%d)", re, to)

	j.printStatus(status)
	j.sock.Connect(j.opts.RouterEndpoint)
}

func (j *jomiel) inquire(uri string) {
	j.send(uri)
	j.recv()
}

func (j *jomiel) send(uri string) {
	inquiry := &msgs.Inquiry{
		Inquiry: &msgs.Inquiry_Media{
			Media: &msgs.MediaInquiry{
				InputUri: uri,
			},
		},
	}

	out, err := proto.Marshal(inquiry)
	if err != nil {
		log.Fatalln("failed to encode inquiry: ", err)
	}

	j.printStatus("<send>")
	j.sock.SendMessage([][]byte{out})
}

func (j *jomiel) recv() {
	/*
	 * NOTE
	 *
	 * Poller.Wait() seems to behave strangely in docker containers.
	 * The function timeouts immediately when the container is run:
	 *  $ docker run --rm -t IMAGE [args...]
	 *
	 * Where as,
	 *  $ docker run --rm -it IMAGE sh
	 *  (container) ./demo [args...]  # Works as expected.
	 *
	 * Previously, with the following setup, Poller.Wait() would
	 * return only after timeout was met:
	 *  - ZeroMQ version 4.2 (czmq version 4.0)
	 *
	 * Whereas, with the following setup, Poller.Wait() returns
	 * immediately with 'sck' set to 'nil' when run in a docker
	 * container (as described above):
	 *  - ZeroMQ version 4.3 (czmq version 4.1)
	 *  - ZeroMQ version 4.3 (czmq version 4.2)
	 */
	poller, err := czmq.NewPoller(j.sock)
	if err != nil {
		log.Fatalln("failed to create a poller: ", err)
	}
	defer poller.Destroy()

	sck, err := poller.Wait(j.opts.ConnectTimeout * 1000)
	if err != nil {
		log.Fatalln("failed to pollin an event: ", err)
	} else {
		if sck == nil {
			log.Fatalln("error: connection timed out")
		}
	}

	data, err := j.sock.RecvMessage()
	if err != nil {
		log.Fatalln("failed to receive message: ", err)
	}

	response := &msgs.Response{}
	err = proto.Unmarshal(data[0], response)
	if err != nil {
		log.Fatalln("failed to decode response: ", err)
	}

	j.dumpResponse(response)
}

func (j *jomiel) printStatus(status string) {
	if !j.opts.BeTerse {
		log.Println("status: " + status)
	}
}

func (j *jomiel) toJson(response *msgs.Response) string {
	json, err := json.MarshalIndent(response, "", "  ")
	if err != nil {
		log.Fatalln("error: ", err)
	}
	return string(json)
}

func (j *jomiel) printMessage(status string, message *msgs.Response) {
	var result string
	if j.opts.OutputJson {
		result = j.toJson(message)
	} else {
		result = proto.MarshalTextString(message)
	}
	j.printStatus(status)
	fmt.Printf(result)
}

func (j *jomiel) dumpResponse(response *msgs.Response) {
	responseStatus := response.GetStatus()
	mediaResponse := response.GetMedia()

	if responseStatus.GetCode() == msgs.StatusCode_STATUS_CODE_OK {
		if j.opts.BeTerse {
			j.dumpTerseResponse(mediaResponse)
		} else {
			j.printMessage("<recv>", response)
		}
	} else {
		j.printMessage("<recv>", response)
	}
}

func (j *jomiel) dumpTerseResponse(response *msgs.MediaResponse) {
	fmt.Println("---\ntitle: " + response.GetTitle())
	fmt.Println("quality:")

	stream := response.GetStream()

	for i := 0; i < len(stream); i++ {
		quality := stream[i].GetQuality()
		fmt.Printf("  profile: %s\n    width: %d\n    height: %d\n",
			quality.GetProfile(), quality.GetWidth(), quality.GetHeight())
	}
}
