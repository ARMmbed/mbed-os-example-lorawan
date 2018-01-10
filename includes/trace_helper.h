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

#ifndef APP_TRACE_HELPER_H_
#define APP_TRACE_HELPER_H_

/**
 * Helper function for the application to setup Mbed trace.
 * It Wouldn't do anything if the FEATURE_COMMON_PAL is not added
 * or if the trace is disabled using mbed_app.json
 */
void setup_trace();

#endif /* APP_TRACE_HELPER_H_ */
