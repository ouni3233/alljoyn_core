/**
 * @file
 * BusObject responsible for implementing the AllJoyn methods (org.alljoyn.Bus)
 * for messages directed to the bus.
 */

/******************************************************************************
 * Copyright 2010-2011, Qualcomm Innovation Center, Inc.
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
#ifndef _ALLJOYN_ALLJOYNOBJ_H
#define _ALLJOYN_ALLJOYNOBJ_H

#include <qcc/platform.h>
#include <vector>

#include <qcc/String.h>
#include <qcc/StringMapKey.h>
#include <qcc/Thread.h>
#include <qcc/time.h>

#include <alljoyn/BusObject.h>
#include <alljoyn/Message.h>

#include "Bus.h"
#include "NameTable.h"
#include "RemoteEndpoint.h"
#include "Transport.h"
#include "VirtualEndpoint.h"


namespace ajn {

/**
 * BusObject responsible for implementing the standard AllJoyn methods at org.alljoyn.Bus
 * for messages directed to the bus.
 */
class AllJoynObj : public BusObject, public NameListener, public TransportListener {
    friend class RemoteEndpoint;

  public:

    /**
     * Constructor
     *
     * @param bus        Bus to associate with org.freedesktop.DBus message handler.
     * @param router     The DaemonRouter associated with the bus.
     */
    AllJoynObj(Bus& bus);

    /**
     * Destructor
     */
    ~AllJoynObj();

    /**
     * Initialize and register this DBusObj instance.
     *
     * @return ER_OK if successful.
     */
    QStatus Init();

    /**
     * Called when object is successfully registered.
     */
    void ObjectRegistered(void);

    /**
     * Respond to a bus request to connect to a remote AllJoyn instance.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   connectSpec  string   The bus address of the remote daemon to connect to.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_CONNECT_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void Connect(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Respond to a bus request to disconnect from a remote AllJoyn instance.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   connectSpec  string   The bus address of the remote daemon to connect to.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_CONNECT_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void Disconnect(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Respond to a bus request to advertise the existence of a remote AllJoyn instance.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   advertisedName  string   A locally obtained well-known name that should be advertised to external AllJoyn instances.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_ADVERTISE_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void AdvertiseName(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Respond to a bus request to cancel a previous advertisement.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   advertisedName  string   A previously advertised well-known name that no longer needs to be advertised.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_ADVERTISE_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void CancelAdvertiseName(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Process incoming ListAdvertisedNames method call from remote daemons.
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void ListAdvertisedNames(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Respond to a bus request to look for advertisements from remote AllJoyn instances.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   namePrefix    string   A well-known name prefix that the caller wants to be notified of (via signal)
     *                          when a remote Bus instance is found that advertises a name that matches the prefix.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_FINDNAME_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void FindName(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Respond to a bus request to cancel a previous (successful) FindName request.
     *
     * The input Message (METHOD_CALL) is expected to contain the following parameters:
     *   namePrefix    string   The well-known name prefix that was used in a successful call to FindName.
     *
     * The output Message (METHOD_REPLY) contains the following parameters:
     *   resultCode   uint32   A ALLJOYN_CANCELFINDNAME_* reply code (see AllJoynStd.h)
     *
     * @param member  Member.
     * @param msg     The incoming message.
     */
    void CancelFindName(const InterfaceDescription::Member* member, Message& msg);

    /**
     * Add a new Bus-to-bus endpoint.
     *
     * @param endpoint  Bus-to-bus endpoint to add.
     * @param ER_OK if successful
     */
    QStatus AddBusToBusEndpoint(RemoteEndpoint& endpoint);

    /**
     * Remove an existing Bus-to-bus endpoint.
     *
     * @param endpoint  Bus-to-bus endpoint to add.
     */
    void RemoveBusToBusEndpoint(RemoteEndpoint& endpoint);

    /**
     * Process incoming ExchangeNames signals from remote daemons.
     *
     * @param member        Interface member for signal
     * @param sourcePath    object path sending the signal.
     * @param msg           The signal message.
     */
    void ExchangeNamesSignalHandler(const InterfaceDescription::Member* member, const char* sourcePath, Message& msg);

    /**
     * Process incoming NameChanged signals from remote daemons.
     *
     * @param member        Interface member for signal
     * @param sourcePath    object path sending the signal.
     * @param msg           The signal message.
     */
    void NameChangedSignalHandler(const InterfaceDescription::Member* member, const char* sourcePath, Message& msg);

    /**
     * NameListener implementation called when a bus name changes ownership.
     *
     * @param alias     Well-known bus name now owned by listener.
     * @param oldOwner  Unique name of old owner of alias or NULL if none existed.
     * @param newOwner  Unique name of new owner of alias or NULL if none (now) exists.
     */
    void NameOwnerChanged(const qcc::String& alias,
                          const qcc::String* oldOwner,
                          const qcc::String* newOwner);

    /**
     * Receive notification of a new bus instance via TransportListener.
     * Internal use only.
     *
     * @param   busAddr   Address of discovered bus.
     * @param   guid      Daemon GUID of discovered bus.
     * @param   names     Vector of bus names advertised by the discovered bus.
     */
    void FoundNames(const qcc::String& busAddr, const qcc::String& guid, const std::vector<qcc::String>* names, uint8_t ttl);

    /**
     * Called when a transport gets a surprise disconnect from a remote bus.
     *
     * @param busAddr       The address of the bus formatted as a string.
     */
    void BusConnectionLost(const qcc::String& busAddr);

  private:
    Bus& bus;                             /**< The bus */
    DaemonRouter& router;                 /**< The router */
    const InterfaceDescription::Member* foundNameSignal;   /**< org.alljoyn.Bus.FoundName signal */
    const InterfaceDescription::Member* lostAdvNameSignal; /**< org.alljoyn.Bus.LostAdvertisdName signal */
    const InterfaceDescription::Member* busConnLostSignal; /**< org.alljoyn.Bus.BusConnectionLost signal */

    /** Map of open connectSpecs to local endpoint name(s) that require the connection. */
    std::multimap<qcc::String, qcc::String> connectMap;
    qcc::Mutex connectMapLock;            /**< Mutex that protects connectMap */

    /** Map of active advertised names to requesting local endpoint name(s) */
    std::multimap<qcc::String, qcc::String> advertiseMap;
    qcc::Mutex advertiseMapLock;          /**< Mutex that protects advertiseMap */

    /** Map of active discovery names to requesting local endpoint name(s) */
    std::multimap<qcc::String, qcc::String> discoverMap;
    qcc::Mutex discoverMapLock;            /**< Mutex that protects discoverMap */

    /** Map of discovered bus names and their guids and busAddrs (protected by discoverMapLock) */
    struct NameMapEntry {
        qcc::String guid;
        qcc::String busAddr;
        uint32_t timestamp;
        uint32_t ttl;
        NameMapEntry(qcc::String guid, qcc::String busAddr, uint32_t ttl) : guid(guid), busAddr(busAddr), timestamp(qcc::GetTimestamp()), ttl(ttl) { }

    };
    std::multimap<qcc::String, NameMapEntry> nameMap;

    const qcc::GUID& guid;               /**< Global GUID of this daemon */

    const InterfaceDescription::Member* exchangeNamesSignal;   /**< org.alljoyn.Bus.ExchangeNames signal member */

    std::map<qcc::String, VirtualEndpoint> virtualEndpoints;   /**< Map of endpoints that reside behind a connected AllJoyn daemon */
    qcc::Mutex virtualEndpointsLock;     /**< Mutex that protects virtualEndpoints map */

    std::map<qcc::StringMapKey, RemoteEndpoint*> b2bEndpoints;    /**< Map of bus-to-bus endpoints that are connected to external daemons */
    qcc::Mutex b2bEndpointsLock;         /**< Mutex that protects b2bEndpoints map */

    /** NameMapReaperThread removes expired names from the nameMap */
    class NameMapReaperThread : public qcc::Thread {
      public:
        NameMapReaperThread(AllJoynObj* ajnObj) : qcc::Thread("NameMapReaper"), ajnObj(ajnObj) { }

        qcc::ThreadReturn STDCALL Run(void* arg);

      private:
        AllJoynObj* ajnObj;
    };

    NameMapReaperThread nameMapReaper;           /**< Removes expired names from nameMap */

    /**
     * Utility function used to send a single FoundName signal.
     *
     * @param dest        Unique name of destination.
     * @param name        Well-known name that was found.
     * @param guid        GUID of the remote bus that was found to be advertising name.
     * @param namePrefix  Well-known name prefix used in call to FindName() that triggered this notification.
     * @param busAddr     The bus addresss of the remote bus.
     * @return ER_OK if succssful.
     */
    QStatus SendFoundAdvertisedName(const qcc::String& dest,
                                    const qcc::String& name,
                                    const qcc::String& guid,
                                    const qcc::String& namePrefix,
                                    const qcc::String& busAddr);

    /**
     * Utility function used to send LostAdvertisedName signals to each "interested" local endpoint.
     *
     * @param name        Well-known name that was found.
     * @param guid        GUID of the remote bus that was found to be advertising name.
     * @param busAddr     The bus addresss of the remote bus.
     * @return ER_OK if succssful.
     */
    QStatus SendLostAdvertisedName(const qcc::String& name,
                                   const qcc::String& guid,
                                   const qcc::String& busAddr);

    /**
     * Add a virtual endpoint with a given unique name.
     *
     * @param uniqueName          The uniqueName of the virtual endpoint.
     * @param busToBusEndpoint    The bus-to-bus endpoint that "owns" the virtual endpoint.
     * @param changesMade         [OUT] Written to true of virtual endpoint was created (as opposed to already existing).
     * @return The virtual endpoint.
     */
    VirtualEndpoint& AddVirtualEndpoint(const qcc::String& uniqueName, RemoteEndpoint& busToBusEndpoint, bool* changesMade = NULL);

    /**
     * Remove a virtual endpoint.
     *
     * @param endpoint   The virtualEndpoint to be removed.
     */
    void RemoveVirtualEndpoint(VirtualEndpoint& endpoint);

    /**
     * Find a virtual endpoint by its name.
     *
     * @param uniqueName    The name of the endpoint to find.
     * @return The requested virtual endpoint or NULL if not found.
     */
    VirtualEndpoint* FindVirtualEndpoint(const qcc::String& uniqueName);

    /**
     * Internal bus-to-bus remote endpoint listener.
     * Called when any virtual endpoint's remote endpoint exits.
     *
     * @param ep   RemoteEndpoint that is exiting.
     */
    void EndpointExit(RemoteEndpoint* ep);

    /**
     * Send signal that informs remote bus of names available on local daemon.
     * This signal is used only in bus-to-bus connections.
     *
     * @param endpoint    Remote endpoint to exchange names with.
     * @return  ER_OK if successful.
     */
    QStatus ExchangeNames(RemoteEndpoint& endpoint);

    /**
     * Process a connect request from a given (locally-connected) endpoint.
     *
     * @param uniqueName         Name of endpoint requesting the disconnection.
     * @param normConnectSpec    Normalized connect spec to be disconnected.
     * @return ER_OK if successful.
     */
    QStatus ProcConnect(const qcc::String& uniqueName, const qcc::String& normConnectSpec, RemoteEndpoint** newep);

    /**
     * Process a disconnect request from a given (locally-connected) endpoint.
     *
     * @param uniqueName         Name of endpoint requesting the disconnection.
     * @param normConnectSpec    Normalized connect spec to be disconnected.
     * @return ER_OK if successful.
     */
    QStatus ProcDisconnect(const qcc::String& uniqueName, const qcc::String& normConnectSpec);

    /**
     * Process a request to cancel advertising a name from a given (locally-connected) endpoint.
     *
     * @param uniqueName         Name of endpoint requesting end of advertising
     * @param advertiseName      Well-known name whose advertising is to be canceled.
     * @return ER_OK if successful.
     */
    QStatus ProcCancelAdvertise(const qcc::String& uniqueName, const qcc::String& advertiseName);

    /**
     * Process a request to cancel discovery of a name prefix from a given (locally-connected) endpoint.
     *
     * @param endpointName         Name of endpoint requesting end of advertising
     * @param namePrefix           Well-known name prefix to be removed from discovery list
     * @return ER_OK if successful.
     */
    QStatus ProcCancelFindName(const qcc::String& endpointName, const qcc::String& namePrefix);

    /**
     * Validate and normalize a transport specification string.  Given a
     * transport specification, convert it into a form which is guaranteed to
     * have a one-to-one relationship with a transport.  (This is just a
     * convenient inline wrapper for internal use).
     *
     * @param inSpec    Input transport connect spec.
     * @param outSpec   [OUT] Normalized transport connect spec.
     * @param argMap    [OUT] Normalized parameter map.
     * @return ER_OK if successful.
     */
    QStatus NormalizeTransportSpec(const char* inSpec,
                                   qcc::String& outSpec,
                                   std::map<qcc::String, qcc::String>& argMap)
    {
        return bus.GetInternal().GetTransportList().NormalizeTransportSpec(inSpec, outSpec, argMap);
    }

    /**
     * Get a list of the currently advertised names
     *
     * @param names  A vector containing the advertised names.
     */
    void GetAdvertisedNames(std::vector<qcc::String>& names);

};

}

#endif
