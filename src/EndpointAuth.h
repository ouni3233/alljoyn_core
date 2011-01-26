/**
 * @file
 * EndpointAuth is a utility class that provides authentication
 * functions for BusEndpoint implementations.
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/
#ifndef _ALLJOYN_ENDPOINTAUTH_H
#define _ALLJOYN_ENDPOINTAUTH_H

#include <qcc/platform.h>

#include <qcc/String.h>
#include <qcc/GUID.h>
#include <qcc/Stream.h>

#include "BusInternal.h"

namespace ajn {


/**
 * Foward declaration of class Bus
 */
class BusAttachment;


/**
 * %EndpointAuth is a utility class responsible for adding endpoint authentication
 * to BusEndpoint implementations.
 */
class EndpointAuth {
  public:

    /**
     * Constructor
     *
     * @param bus          Bus for which authentication is done
     * @param stream       Stream used to communicate with media.
     * @param isAcceptor   Indicates if the endpoint is the acceptor of a connection (default is false).
     */
    EndpointAuth(BusAttachment& bus, qcc::Stream& stream, bool isAcceptor = false) :
        bus(bus),
        stream(stream),
        uniqueName(bus.GetInternal().GetRouter().GenerateUniqueName()),
        isAccepting(isAcceptor),
        remoteProtocolVersion(0)
    { }

    /**
     * Destructor
     */
    ~EndpointAuth() { };

    /**
     * Establish a connection.
     *
     * @param authMechanisms  The authentication mechanisms to try.
     * @param authUsed        Returns the name of the authentication method that was used to establish the connection.
     * @param isBusToBus      [IN,OUT] When initiating connection indicate [IN] whether this is a bus-to-bus connection.
     *                        When responding to a connection attempt indicate [OUT] whether this is a bus-to-bus connection.
     * @param allowRemote     [IN,OUT] When initiating a connection this input tells the local daemon whether it wants to receive
     *                        messages from remote busses.
     *                        When accepting a connection, this output indicates whether the connected endpoint is willing to receive
     *                        messages from remote busses.
     * @return
     *      - ER_OK if successful
     *      - An error status otherwise
     */
    QStatus Establish(const qcc::String& authMechanisms, qcc::String& authUsed, bool& isBusToBus, bool& allowRemote);

    /**
     * Get the unique bus name assigned by the bus for this endpoint.
     *
     * @return
     *      - unique bus name
     *      - empty string if called before the endpoint has been authenticated.
     */
    const qcc::String& GetUniqueName() const { return uniqueName; }

    /**
     * Get the bus name for the peer at the remote end of this endpoint. If we are on the initiating
     * side of the connection this is the bus name of the responder and if we are the responder this
     * is the bus name of the initiator.
     *
     * @return
     *      - bus name of the remote side.
     *      - empty string if called before the endpoint has been authenticated.
     */
    const qcc::String& GetRemoteName() const { return remoteName; }

    /**
     * Get the GUID of the remote side.
     *
     * @return   The GUID for the remote side of this connection.
     *
     */
    const qcc::GUID& GetRemoteGUID() const { return remoteGUID; }

    /**
     * Get the AllJoyn protocol version number of the remote side.
     *
     * @return   The AllJoyn protocol version of the remote side.
     *
     */
    uint32_t GetRemoteProtocolVersion() const { return remoteProtocolVersion; }

  private:

    EndpointAuth();

    BusAttachment& bus;
    qcc::Stream& stream;             ///< Stream connection to peer bus node
    qcc::String uniqueName;          ///< Unique bus name for endpoint
    qcc::String remoteName;          ///< Bus name for the peer at other end of this endpoint

    bool isAccepting;                ///< Indicates if this is a client or server

    qcc::GUID remoteGUID;            ///< GUID of the remote side (when applicable)
    uint32_t remoteProtocolVersion;  ///< ALLJOYN protocol version of the remote side

    /* Internal methods */

    QStatus Hello(bool isBusToBus, bool allowRemote);
    QStatus WaitHello(bool& isBusToBus, bool& allowRemote);
};

}

#endif
