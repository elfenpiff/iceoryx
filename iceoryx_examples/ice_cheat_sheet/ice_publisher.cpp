// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <atomic>
#include <chrono>
#include <thread>

std::atomic_bool keepRunning{true};
uint32_t myData;
uint32_t myInitialData;

void sendDataWithErrorHandling()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        myPublisher.loan(sizeof(CounterTopic))
            .and_then([](iox::popo::Sample<void>& sample) {
                static_cast<CounterTopic*>(sample.get())->data = myData;
                sample.publish();
            })
            .or_else([](const iox::popo::AllocationError& errorValue) {
                // perform error handling
            });
    }

    myPublisher.stopOffer();
}

void sendDataWithErrorHandlingOldStyle()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        auto sample = myPublisher.loan(sizeof(CounterTopic));
        if (sample.has_error())
        {
            // perform error handling
        }
        else
        {
            static_cast<CounterTopic*>(sample->get())->data = myData;
            myPublisher.publish(std::move(*sample));
        }
    }

    myPublisher.stopOffer();
}

void sendDataOnlyWithSubscribers()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        if (myPublisher.hasSubscribers())
        {
            myPublisher.loan(sizeof(CounterTopic)).and_then([](iox::popo::Sample<void>& sample) {
                static_cast<CounterTopic*>(sample.get())->data = myData;
                sample.publish();
            });
        }
    }

    myPublisher.stopOffer();
}

void dismissDataWithoutSending()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        myPublisher.loan(sizeof(CounterTopic));
    }

    myPublisher.stopOffer();
}

void acquirePreviousSendData()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    myPublisher.loan(sizeof(CounterTopic)).and_then([](iox::popo::Sample<void>& sample) {
        static_cast<CounterTopic*>(sample.get())->data = myData;
        sample.publish();
    });

    while (keepRunning)
    {
        myPublisher.loanPreviousSample().and_then([&](iox::popo::Sample<void>& sample) {
            static_cast<CounterTopic*>(sample.get())->data = myData;
            sample.publish();
        });
    }

    myPublisher.stopOffer();
}

int main()
{
    // adjust this line to the example you would like to have
    auto example = sendDataWithErrorHandling;

    std::thread exampleThread(example);
    std::thread stopMeLater([&] {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        keepRunning.store(false);
    });

    stopMeLater.join();
    exampleThread.join();

    return (EXIT_SUCCESS);
}
