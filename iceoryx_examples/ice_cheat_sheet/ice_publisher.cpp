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

#include <algorithm>
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
        {
            auto sample = myPublisher.loan(sizeof(CounterTopic));
        } // sample goes out of scope and cleans itself up without sending
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
        myPublisher
            .loanPreviousSample()
            // got previous sample, only change updated values
            .and_then([&](iox::popo::Sample<void>& sample) {
                static_cast<CounterTopic*>(sample.get())->hugeData[2] = 42;
                sample.publish();
            })
            // someone else is reading the last sample therefore we have to set the whole sample again
            .or_else([&] {
                myPublisher.loan(sizeof(CounterTopic)).and_then([](iox::popo::Sample<void>& sample) {
                    static_cast<CounterTopic*>(sample.get())->data = myData;
                    sample.publish();
                });
            });
    }

    myPublisher.stopOffer();
}

void dynamicSend()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::UntypedPublisher myPublisher({"MyService", "MyInstance", "MyEvent"});

    myPublisher.offer();
    uint64_t counter = 0U;

    while (keepRunning)
    {
        if (myPublisher.isOffered())
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
        ++counter;
        if (counter % 10U == 0U)
        {
            (myPublisher.isOffered()) ? myPublisher.stopOffer() : myPublisher.offer();
        }
    }

    if (myPublisher.isOffered())
    {
        myPublisher.stopOffer();
    }
}

void multiplePublishersSendData()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");

    constexpr uint64_t nPub = 3;
    iox::cxx::vector<iox::popo::UntypedPublisher, nPub> myPublisher;
    myPublisher.emplace_back(iox::capro::ServiceDescription{"MyService", "MyInstance1", "MyEvent"});
    myPublisher.emplace_back(iox::capro::ServiceDescription{"MyService", "MyInstance2", "MyEvent"});
    myPublisher.emplace_back(iox::capro::ServiceDescription{"MyService", "MyInstance3", "MyEvent"});

    std::for_each(myPublisher.begin(), myPublisher.end(), [](iox::popo::UntypedPublisher& pub) { pub.offer(); });

    while (keepRunning)
    {
        for (auto& pub : myPublisher)
        {
            pub.loan(sizeof(CounterTopic))
                .and_then([](iox::popo::Sample<void>& sample) {
                    static_cast<CounterTopic*>(sample.get())->data = myData;
                    sample.publish();
                })
                .or_else([](const iox::popo::AllocationError& errorValue) {
                    // perform error handling
                });
        }
    }

    std::for_each(myPublisher.begin(), myPublisher.end(), [](iox::popo::UntypedPublisher& pub) { pub.stopOffer(); });
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
