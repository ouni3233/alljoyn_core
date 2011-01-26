/*
 * Copyright 2010 - 2011, Qualcomm Innovation Center, Inc.
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
 *
 */
package org.alljoyn.bus.daemonservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

//
// We expect that clients of the Android Bus Daemon Service will ask the
// Android runtime to instantiate the service using an Intent.  When the
// daemon service is installed (cf. adb install), the Android system notices
// that our service has the following lines in its AndroidManifest.xml:
//
//  <intent-filter>
//      <category android:name="android.intent.category.DEFAULT" />
//      <action android:name="org.alljoyn.bus.daemonservice.START_DAEMON" />
//  </intent-filter>
//
// This tells the system that our Service can respond to the START_DAEMON
// action.  We expect an application Activity to create an Intent with an
// action of that name and ask for a service to be instantiated that can
// deal with that action, as in the following code:
//
//     Intent intent = new Intent("org.alljoyn.bus.daemonservice.START_DAEMON");
//     this.startService(intent);
//
// Since we declare android:process="bbdaemon" in our manifest, the Android
// system will, in response to the startService() call, create a process for
// our service, launch it and call our onStartCommand() method.  (We require a
// 2.0 or greater platform to enable this behavior so we can return START_STICKY.
// This means that our service will be explicitly stopped and will therefore
// run for arbitrary lengths of time.
//
// Once a service has been started, the Android system will try to keep it
// running.  If we return START_STICKY, the service will be kept around until
// the system needs to kill it because of a low memory situation.
//
// The Android kernel includes what is called a Low Memory Killer that takes
// requests to reduce memory consumption.  For each request, it looks at a
// least recently used (LRU) process list and sends a SIGKILL to what it
// decides is the least important process based on what the process is doing.
// The upshot of this is that our service will really get no indication that
// it has been chosen as a victim for termination.  It will just be killed as
// dead as it would if the super-user did a kill -9.  Because of this, we
// don't bother with any user-land way to end the service.  We just expect
// end our lifecycle by being killed one way or another.
//
// The rules to decide which process is least important, and therefore when
// the service might be killed by the Low Memory Killer are fairly complex
// and follow a priority-based scheme.  The Android framework sends hints
// about the importance of a process through the oom_adj value of the
// process (this is a Linux thing -- see the Linux OOM Killer).
//
// Once a service has been started, it is considered less important than
// an application process that is visible to the user, but more important
// than non-visible application processes.  Since there are relatively few
// visible applications, services will generally not be killed except in
// *extreme* low memory situations.
//
// If this isn't good enough, there are a couple of ways to gain even more
// priority, though.  If the service puts itself into a foreground state
// by calling startForeground(), it is essentially immune from being killed.
// If a client binds to a service (this does put our existence in the hands
// of the client) the service inherits the clients visibility -- that
// is, the service is never less important than the most important client.
//
// The bottom line is that we really don't have to worry about being killed
// due to low memory problems.  Unused applications will go first.  If we
// do get killed, we will be whacked with SIGKILL and there's nothing we
// can do anyway since we can never know what is happening.
//
public class DaemonService extends Service {
    //
    // Load the JNI library that holds all of the AllJoyn code.  The test program,
    // bbdaemon has been made into a static library that exports the function
    // DaemonMain() instead of being an executable with a main() entry point.
    // Our JNI code links against this static library and produces a dynamic
    // library that exports a function (runDaemon) that simply calls into the
    // static libbbdaemon-lib.a library.  This effectively runs bbdaemon with
    // its default parameters when the Android system detects a need for the
    // "org.alljoyn.bus.daemonservice.START_DAEMON" Action to be serviced.
    //
    static {
        System.loadLibrary("bus-daemon-jni");
    }

    //
    // Declaration of the JNI function that will actually run the daemon.
    //
    public static native int runDaemon();

    //
    // Execute the daemon main "program" in a thread in case we somehow get
    // called in the context of a GUI.
    //
    private DaemonThread thread;

    //
    // Just run the daemon.  We expect this call never to return.  The service
    // will eventually end its life-cycle due to a SIGKILL from the Android
    // system (perhaps due to the low memory killer, perhaps due to another
    // kill request).
    //
    class DaemonThread extends Thread {

        public void run()
        {
            runDaemon();
        }
    }

    //
    // This method is called the first time the service is launched.  We just
    // spin up a thread to run the bbdaemon library main function and return.
    //
    @Override
    public void onCreate() {
        super.onCreate();
        thread = new DaemonThread();
        thread.start();
    }

    //
    // We need to override this method so we can return START_STICKY.  This
    // may be called multiple times whenever a new service user pops up.
    //
    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        //
        // START_STICKY means that the service is expected to run for arbitrary
        // lengths of time and will be explicitly started and stopped.
        //
        return START_STICKY;
    }

    //
    // This is where to add code to enable the priority bump due to a bound
    // foreground application.  We don't have any reason to exchange any RPC
    // calls, but the simple act of binding implies that we get that priority
    // bump.
    //
    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }
}
