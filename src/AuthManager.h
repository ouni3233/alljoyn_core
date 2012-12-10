#ifndef _ALLJOYN_AUTHMANAGER_H
#define _ALLJOYN_AUTHMANAGER_H
/**
 * @file
 * This file defines the authentication mechanism manager
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

#ifndef __cplusplus
#error Only include AuthManager.h in C++ code.
#endif

#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/StringMapKey.h>
#include <qcc/Debug.h>

#include <map>

#include "AuthMechanism.h"
#include "KeyStore.h"

#include <alljoyn/Status.h>

#define QCC_MODULE "ALLJOYN_AUTH"

namespace ajn {

/**
 * This class manages authentication mechanisms
 */
class AuthManager {
  public:

    /**
     * Constructor
     */
    AuthManager(KeyStore& keyStore) : keyStore(keyStore) { }

    /**
     * Type for an factory for an authentication mechanism. AllJoynAuthentication mechanism classes
     * provide a function of this type when registering with the AllJoynAuthentication mechanism manager.
     *
     * @param keyStore  The key store for keys and other security credentials required for the
     *                  authentication mechansim.
     *
     * @param listener  Provides callouts for authentication mechanisms that interact with the user or
     *                  application.
     */
    typedef AuthMechanism* (*AuthMechFactory)(KeyStore& keyStore, ProtectedAuthListener& listener);

    /**
     * Registers an authentication mechanism factory function and associates it with a
     * specific authentication mechanism name.
     */
    void RegisterMechanism(AuthMechFactory factory, const char* mechanismName)
    {
        authMechanisms[mechanismName] = factory;
    }

    /**
     * Unregisters an authentication mechanism factory function
     */
    void UnregisterMechanism(const char* mechanismName)
    {
        AuthMechFactoryMap::iterator it = authMechanisms.find(mechanismName);
        if (it != authMechanisms.end()) {
            authMechanisms.erase(it);
        }
    }

    /**
     * Filter out mechanisms with names not listed in the string.
     */
    size_t FilterMechanisms(const qcc::String& list)
    {
        size_t num = 0;
        std::map<qcc::StringMapKey, AuthMechFactory>::iterator it;

        for (it = authMechanisms.begin(); it != authMechanisms.end(); it++) {
            if (list.find(it->first.c_str()) == qcc::String::npos) {
                authMechanisms.erase(it);
                it = authMechanisms.begin();
                num = 0;
            } else {
                ++num;
            }
        }
        return num;
    }

    /**
     * Check that the list of names are registered mechanisms.
     */
    QStatus CheckNames(qcc::String list)
    {
        QStatus status = ER_OK;
        while (!list.empty()) {
            size_t pos = list.find_first_of(' ');
            qcc::String name = list.substr(0, pos);
            if (!authMechanisms.count(name)) {
                status = ER_BUS_INVALID_AUTH_MECHANISM;
                QCC_LogError(status, ("Unknown authentication mechanism %s", name.c_str()));
                break;
            }
            list.erase(0, (pos == qcc::String::npos) ? pos : pos + 1);
        }
        return status;
    }

    /**
     * Returns an authentication mechanism object for the requested authentication mechanism.
     *
     * @param mechanismName String representing the name of the authentication mechanism
     *
     * @param listener  Required for authentication mechanisms that interact with the user or
     *                  application.
     *
     * @return An object that implements the requested authentication mechanism or NULL if there is
     *         no such object. Note this function will also return NULL if the authentication
     *         mechanism requires a listener and none has been provided.
     */
    AuthMechanism* GetMechanism(const qcc::String& mechanismName, ProtectedAuthListener& listener)
    {
        std::map<qcc::StringMapKey, AuthMechFactory>::iterator it = authMechanisms.find(mechanismName);
        if (authMechanisms.end() != it) {
            return (it->second)(keyStore, listener);
        } else {
            return NULL;
        }
    }


  private:

    /**
     * Reference to the keyStore
     */
    KeyStore& keyStore;

    /**
     * Maps authentication mechanisms names to factory functions
     */
    typedef std::map<qcc::StringMapKey, AuthMechFactory> AuthMechFactoryMap;
    AuthMechFactoryMap authMechanisms;
};

}

#undef QCC_MODULE

#endif
