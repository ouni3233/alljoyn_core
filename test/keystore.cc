
/**
 * @file
 *
 * This file tests the keystore and ketyblob functionality
 */

/******************************************************************************
 * $Revision: 26/1 $
 *
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

#include <qcc/platform.h>

#include <qcc/Crypto.h>
#include <qcc/Debug.h>
#include <qcc/FileStream.h>
#include <qcc/KeyBlob.h>
#include <qcc/Pipe.h>
#include <qcc/StringUtil.h>
#include <qcc/Util.h>
#include <qcc/GUID.h>
#include <qcc/time.h>

#include <alljoyn/version.h>
#include "KeyStore.h"

#include <Status.h>

using namespace qcc;
using namespace std;
using namespace ajn;

static const char testData[] = "This is the message that we are going to encrypt and then decrypt and verify";

int main(int argc, char** argv)
{
    qcc::GUID guid1;
    qcc::GUID guid2;
    QStatus status = ER_OK;
    KeyBlob key;

    printf("AllJoyn Library version: %s\n", ajn::GetVersion());
    printf("AllJoyn Library build info: %s\n", ajn::GetBuildInfo());

    Crypto_AES::Block* encrypted = new Crypto_AES::Block[Crypto_AES::NumBlocks(sizeof(testData))];

    /* Encryption step */
    {
        FileSink sink("keystore_test");

        /*
         * Generate a random key
         */
        key.Rand(Crypto_AES::AES128_SIZE, KeyBlob::AES);
        printf("Key %d in  %s\n", key.GetType(), BytesToHexString(key.GetData(), key.GetSize()).c_str());
        /*
         * Encrypt our test string
         */
        Crypto_AES aes(key, Crypto_AES::ENCRYPT);
        status = aes.Encrypt(testData, sizeof(testData), encrypted, Crypto_AES::NumBlocks(sizeof(testData)));
        if (status != ER_OK) {
            printf("Encrypt failed\n");
            goto ErrorExit;
        }
        /*
         * Write the key to a stream
         */
        status = key.Store(sink);
        if (status != ER_OK) {
            printf("Failed to store key\n");
            goto ErrorExit;
        }
        /*
         * Set expiration and write again
         */
        qcc::Timespec expires(1000, qcc::TIME_RELATIVE);
        key.SetExpiration(expires);
        status = key.Store(sink);
        if (status != ER_OK) {
            printf("Failed to store key with expiration\n");
            goto ErrorExit;
        }
        /*
         * Set tag and write again
         */
        key.SetTag("My Favorite Key");
        status = key.Store(sink);
        if (status != ER_OK) {
            printf("Failed to store key with tag\n");
            goto ErrorExit;
        }

        key.Erase();
    }

    /* Decryption step */
    {
        FileSource source("keystore_test");

        /*
         * Read the key from a stream
         */
        KeyBlob inKey;
        status = inKey.Load(source);
        if (status != ER_OK) {
            printf("Failed to load key\n");
            goto ErrorExit;
        }
        printf("Key %d out %s\n", inKey.GetType(), BytesToHexString(inKey.GetData(), inKey.GetSize()).c_str());
        /*
         * Decrypt and verify the test string
         */
        {
            char* out = new char[sizeof(testData)];
            Crypto_AES aes(inKey, Crypto_AES::DECRYPT);
            status = aes.Decrypt(encrypted, Crypto_AES::NumBlocks(sizeof(testData)), out, sizeof(testData));
            if (status != ER_OK) {
                printf("Encrypt failed\n");
                goto ErrorExit;
            }
            if (strcmp(out, testData) != 0) {
                printf("Encryt/decrypt of test data failed\n");
                goto ErrorExit;
            }
        }
        /*
         * Read the key with expiration
         */
        status = inKey.Load(source);
        if (status != ER_OK) {
            printf("Failed to load key with expiration\n");
            goto ErrorExit;
        }
        /*
         * Read the key with tag
         */
        status = inKey.Load(source);
        if (status != ER_OK) {
            printf("Failed to load key with tag\n");
            goto ErrorExit;
        }
        if (inKey.GetTag() != "My Favorite Key") {
            printf("Tag was incorrect\n");
            goto ErrorExit;
        }
    }

    {
        KeyStore keyStore("keystore test");

        keyStore.Load(NULL);
        keyStore.Clear();

        key.Rand(Crypto_AES::AES128_SIZE, KeyBlob::AES);
        keyStore.AddKey(guid1, key);
        key.Rand(620, KeyBlob::GENERIC);
        keyStore.AddKey(guid2, key);

        status = keyStore.Store();
        if (status != ER_OK) {
            printf("Failed to store keystore\n");
            goto ErrorExit;
        }
    }
    {
        KeyStore keyStore("keystore test");
        keyStore.Load(NULL);

        status = keyStore.GetKey(guid1, key);
        if (status != ER_OK) {
            printf("Failed to load guid1\n");
            goto ErrorExit;
        }
        status = keyStore.GetKey(guid1, key);
        if (status != ER_OK) {
            printf("Failed to load guid2\n");
            goto ErrorExit;
        }
    }

    printf("keystore unit test PASSED\n");
    return 0;

ErrorExit:

    printf("keystore unit test FAILED %s\n", QCC_StatusText(status));

    return -1;
}
