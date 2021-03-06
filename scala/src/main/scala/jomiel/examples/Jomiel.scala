/*
 * -*- coding: utf-8 -*-
 *
 * jomiel-client-demos
 *
 * Copyright
 *  2021 Toni Gündoğdu
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package jomiel.client.demos

import jomiel.protobuf.v1beta1.media.{MediaInquiry, MediaResponse}
import jomiel.protobuf.v1beta1.message.Inquiry.Inquiry.Media
import jomiel.protobuf.v1beta1.message.{Inquiry, Response}
import jomiel.protobuf.v1beta1.status.StatusCode.STATUS_CODE_OK
import org.tinylog.scala.Logger.{error, info}
import org.zeromq.SocketType.REQ
import org.zeromq.ZContext
import org.zeromq.ZMQ.Poller.POLLIN
import scalapb.GeneratedMessage
import scalapb.json4s.JsonFormat.toJsonString

import java.lang.System.exit

class Jomiel(opts: Runner) {
  private val ctx = new ZContext()
  private val sck = ctx.createSocket(REQ)
  private val poller = ctx.createPoller(1)

  sck.setLinger(0)
  poller.register(sck, POLLIN)

  def inquire(): Unit = {
    if (!opts.uri.isEmpty) {
      connect()
      opts.uri.forEach { uri =>
        sendInquiry(uri)
        receiveResponse()
      }
    } else {
      error("error: input URI not given")
      exit(1)
    }
  }

  private def connect(): Unit = {
    val re = opts.routerEndpoint
    val to = opts.connectTimeout
    printStatus(s"<connect> $re (timeout=$to)")
    sck.connect(re)
  }

  private def sendInquiry(uri: String): Unit = {
    val msg = Inquiry(
      inquiry = Media(
        MediaInquiry(inputUri = uri)
      )
    )
    if (!opts.beTerse) printMessage("<send>", msg)
    sck.send(msg.toByteArray)
  }

  private def receiveResponse(): Unit = {
    poller.poll(opts.connectTimeout * 1000)
    if (poller.pollin(0)) {
      val bytes = sck.recv()
      val msg = Response.parseFrom(bytes)
      dumpResponse(msg)
    } else {
      error("error: connection timed out")
      exit(1)
    }
  }

  private def printStatus(status: String): Unit = {
    if (!opts.beTerse) error(s"status: $status")
  }

  private def dumpResponse(msg: Response): Unit = {
    val status = "<recv>"
    if (msg.status.get.code == STATUS_CODE_OK) {
      if (opts.beTerse) dumpTerseResponse(msg.getMedia)
      else printMessage(status, msg.getMedia)
    } else printMessage(status, msg)
  }

  private def dumpTerseResponse(msg: MediaResponse): Unit = {
    info(s"---\ntitle: ${msg.title}\nquality:")
    msg.stream.foreach(stream =>
      info(s"  profile: ${stream.quality.get.profile}\n"
        + s"    width: ${stream.quality.get.width}\n"
        + s"    height: ${stream.quality.get.height}"
      )
    )
  }

  private def printMessage(status: String, msg: GeneratedMessage): Unit = {
    val result =
      if (opts.outputJson) toJsonString(msg)
      else msg.toString
    printStatus(status)
    info(result)
  }
}
