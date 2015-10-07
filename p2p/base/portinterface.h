/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_P2P_BASE_PORTINTERFACE_H_
#define WEBRTC_P2P_BASE_PORTINTERFACE_H_

#include <string>

#include "webrtc/p2p/base/transport.h"
#include "webrtc/base/socketaddress.h"

namespace rtc {
class Network;
struct PacketOptions;
}

namespace cricket {
class Connection;
class IceMessage;
class StunMessage;

enum ProtocolType {
  PROTO_UDP,
  PROTO_TCP,
  PROTO_SSLTCP,
  PROTO_LAST = PROTO_SSLTCP
};

// Defines the interface for a port, which represents a local communication
// mechanism that can be used to create connections to similar mechanisms of
// the other client. Various types of ports will implement this interface.
class PortInterface {
 public:
  virtual ~PortInterface() {}

  virtual const std::string& Type() const = 0;
  virtual rtc::Network* Network() const = 0;

  // Methods to set/get ICE role and tiebreaker values.
  virtual void SetIceRole(IceRole role) = 0;
  virtual IceRole GetIceRole() const = 0;

  virtual void SetIceTiebreaker(uint64_t tiebreaker) = 0;
  virtual uint64_t IceTiebreaker() const = 0;

  virtual bool SharedSocket() const = 0;

  // PrepareAddress will attempt to get an address for this port that other
  // clients can send to.  It may take some time before the address is ready.
  // Once it is ready, we will send SignalAddressReady.  If errors are
  // preventing the port from getting an address, it may send
  // SignalAddressError.
  virtual void PrepareAddress() = 0;

  // Returns the connection to the given address or NULL if none exists.
  virtual Connection* GetConnection(
      const rtc::SocketAddress& remote_addr) = 0;

  // Creates a new connection to the given address.
  enum CandidateOrigin { ORIGIN_THIS_PORT, ORIGIN_OTHER_PORT, ORIGIN_MESSAGE };
  virtual Connection* CreateConnection(
      const Candidate& remote_candidate, CandidateOrigin origin) = 0;

  // Functions on the underlying socket(s).
  virtual int SetOption(rtc::Socket::Option opt, int value) = 0;
  virtual int GetOption(rtc::Socket::Option opt, int* value) = 0;
  virtual int GetError() = 0;

  virtual const std::vector<Candidate>& Candidates() const = 0;

  // Sends the given packet to the given address, provided that the address is
  // that of a connection or an address that has sent to us already.
  virtual int SendTo(const void* data, size_t size,
                     const rtc::SocketAddress& addr,
                     const rtc::PacketOptions& options, bool payload) = 0;

  // Indicates that we received a successful STUN binding request from an
  // address that doesn't correspond to any current connection.  To turn this
  // into a real connection, call CreateConnection.
  sigslot::signal6<PortInterface*, const rtc::SocketAddress&,
                   ProtocolType, IceMessage*, const std::string&,
                   bool> SignalUnknownAddress;

  // Sends a response message (normal or error) to the given request.  One of
  // these methods should be called as a response to SignalUnknownAddress.
  // NOTE: You MUST call CreateConnection BEFORE SendBindingResponse.
  virtual void SendBindingResponse(StunMessage* request,
                                   const rtc::SocketAddress& addr) = 0;
  virtual void SendBindingErrorResponse(
      StunMessage* request, const rtc::SocketAddress& addr,
      int error_code, const std::string& reason) = 0;

  // Signaled when this port decides to delete itself because it no longer has
  // any usefulness.
  sigslot::signal1<PortInterface*> SignalDestroyed;

  // Signaled when Port discovers ice role conflict with the peer.
  sigslot::signal1<PortInterface*> SignalRoleConflict;

  // Normally, packets arrive through a connection (or they result signaling of
  // unknown address).  Calling this method turns off delivery of packets
  // through their respective connection and instead delivers every packet
  // through this port.
  virtual void EnablePortPackets() = 0;
  sigslot::signal4<PortInterface*, const char*, size_t,
                   const rtc::SocketAddress&> SignalReadPacket;

  virtual std::string ToString() const = 0;

 protected:
  PortInterface() {}
};

}  // namespace cricket

#endif  // WEBRTC_P2P_BASE_PORTINTERFACE_H_
