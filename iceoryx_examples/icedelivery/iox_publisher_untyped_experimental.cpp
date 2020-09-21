// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "topic_data.hpp"

#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <chrono>
#include <iostream>
#include <thread>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

int main(int argc, char* argv[])
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::getInstance("/iox-ex-publisher-modern");

    auto untypedPublisher = iox::popo::UntypedPublisher({"Odometry", "Position", "Vehicle"});
    untypedPublisher.offer();

    float_t ct = 0.0;
    while (!killswitch)
    {
        untypedPublisher.loan(sizeof(Position)).and_then([&](iox::popo::Sample<void>& sample) {
            ++ct;
            new (sample.get()) Position(ct, ct, ct);
            sample.publish();
        });
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
