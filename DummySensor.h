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

#ifndef MBED_LORAWAN_DUMMYSENSOR_H_
#define MBED_LORAWAN_DUMMYSENSOR_H_

/*
 * A dummy sensor for Mbed LoRa Test Application
 */
class DS1820 {
public:
    DS1820(uint32_t)
    {
        value = 1;
    };
    bool begin()
    {
        return true;
    };
    void startConversion() {};
    int32_t read()
    {
        value += 2;
        return value;
    }

private:
    int32_t value;
};



#endif /* MBED_LORAWAN_DUMMYSENSOR_H_ */
