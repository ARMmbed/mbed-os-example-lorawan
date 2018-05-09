/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * If we have tracing library available, we can see traces from within the
 * stack. The library could be made unavailable by removing FEATURE_COMMON_PAL
 * from the mbed_app.json to save RAM.
 */
#if defined(FEATURE_COMMON_PAL)

    #include "platform/PlatformMutex.h"
    #include "mbed_trace.h"

    /**
     * Local mutex object for synchronization
     */
    static PlatformMutex mutex;

    static void serial_lock();
    static void serial_unlock();

    /**
     * Sets up trace for the application
     * Wouldn't do anything if the FEATURE_COMMON_PAL is not added
     * or if the trace is disabled using mbed_app.json
     */
    void setup_trace()
    {
        // setting up Mbed trace.
        mbed_trace_mutex_wait_function_set(serial_lock);
        mbed_trace_mutex_release_function_set(serial_unlock);
        mbed_trace_init();
    }

    /**
     * Lock provided for serial printing used by trace library
     */
    static void serial_lock()
    {
        mutex.lock();
    }

    /**
     * Releasing lock provided for serial printing used by trace library
     */
    static void serial_unlock()
    {
        mutex.unlock();
    }

#else

    void setup_trace()
    {
    }

#endif

