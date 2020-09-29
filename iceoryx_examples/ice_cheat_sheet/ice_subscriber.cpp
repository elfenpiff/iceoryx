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

#include "iceoryx_posh/experimental/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

std::atomic_bool keepRunning{true};

void receiveDataWithErrorHandling()
{
    iox::runtime::PoshRuntime::getInstance("/myAppName");
    iox::popo::UntypedSubscriber mySubscriber({"MyService", "MyInstance", "MyEvent"});
    mySubscriber.subscribe(10);

    while (keepRunning)
    {
        mySubscriber.receive()
            .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& maybeSample) {
                maybeSample.and_then([](iox::popo::Sample<const void>& sample) {
                    std::cout << "Receiving: " << static_cast<const CounterTopic*>(sample.get())->data << std::endl;
                });
            })
            .or_else([](const iox::popo::ChunkReceiveError& errorValue) {
                // perform error handling
            });
    }
    mySubscriber.unsubscribe();
}

void receiveDataWithErrorHandlingOldStyle()
{
    iox::runtime::PoshRuntime::getInstance("/myAppName");
    iox::popo::UntypedSubscriber mySubscriber({"MyService", "MyInstance", "MyEvent"});
    mySubscriber.subscribe(10);

    while (keepRunning)
    {
        auto maybeSample = mySubscriber.receive();
        if (maybeSample.has_error())
        {
            // perform error handling
        }
        else
        {
            auto sample = std::move(maybeSample.get_value());
            if (sample.has_value())
            {
                std::cout << "Receiving: " << static_cast<const CounterTopic*>(sample->get())->data << std::endl;
            }
        }
    }

    mySubscriber.unsubscribe();
}

void receiveDataAfterSubscriptionStateCheck()
{
    iox::runtime::PoshRuntime::getInstance("/myAppName");
    iox::popo::UntypedSubscriber mySubscriber({"MyService", "MyInstance", "MyEvent"});
    mySubscriber.subscribe(10);

    while (keepRunning)
    {
        if (iox::SubscribeState::SUBSCRIBED == mySubscriber.getSubscriptionState())
        {
            mySubscriber.receive()
                .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& maybeSample) {
                    maybeSample.and_then([](iox::popo::Sample<const void>& sample) {
                        std::cout << "Receiving: " << static_cast<const CounterTopic*>(sample.get())->data << std::endl;
                    });
                })
                .or_else([](const iox::popo::ChunkReceiveError& errorValue) {
                    // perform error handling
                });
        }
        else
        {
            std::cout << "Service is not available." << std::endl;
        }
    }

    mySubscriber.unsubscribe();
}

void subscribeAfterFindService()
{
    iox::runtime::InstanceContainer instanceContainer;
    iox::capro::IdString service = "MyService";
    iox::runtime::PoshRuntime::getInstance("/myAppName")
        .findService({service, iox::capro::AnyInstanceString}, instanceContainer)
        .and_then([instanceContainer, service]() {
            if (!instanceContainer.empty())
            {
                iox::popo::UntypedSubscriber mySubscriber({service, instanceContainer[0], "MyEvent"});
                mySubscriber.subscribe(10);
                while (keepRunning)
                {
                    if (iox::SubscribeState::SUBSCRIBED == mySubscriber.getSubscriptionState())
                    {
                        mySubscriber.receive()
                            .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& maybeSample) {
                                maybeSample.and_then([](iox::popo::Sample<const void>& sample) {
                                    std::cout << "Receiving: " << static_cast<const CounterTopic*>(sample.get())->data
                                              << std::endl;
                                });
                            })
                            .or_else([](const iox::popo::ChunkReceiveError& errorValue) {
                                // perform error handling
                            });
                    }
                    else
                    {
                        std::cout << "Service is not available." << std::endl;
                    }
                }
                mySubscriber.unsubscribe();
            }
        })
        .or_else([](iox::Error) {
            // perform error handling
        });
}

int main()
{
    // adjust this line to the example you would like to have
    auto example = receiveDataWithErrorHandling;

    std::thread exampleThread(example);
    std::thread stopMeLater([&] {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        keepRunning.store(false);
    });

    stopMeLater.join();
    exampleThread.join();

    return (EXIT_SUCCESS);
}
