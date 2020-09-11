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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <atomic>
#include <chrono>
#include <thread>

std::atomic_bool keepRunning{true};
uint32_t myData;
uint32_t myInitialData;

void sendData()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }

    myPublisher.stopOffer();
}

void sendDataOnlyWithSubscribers()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));
        sample->data = myData;
        if (myPublisher.hasSubscribers())
        {
            myPublisher.sendChunk(sample);
        }
        else
        {
            myPublisher.freeChunk(sample);
        }
    }

    myPublisher.stopOffer();
}

void sendDataWithCustomHeader()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        auto header = myPublisher.allocateChunkWithHeader(sizeof(CounterTopic));

        header->m_info.m_externalSequenceNumber_bl = true;
        header->m_info.m_sequenceNumber = 42;
        header->m_info.m_usedSizeOfChunk = 42;
        header->m_info.m_totalSizeOfChunk = 42;
        header->m_info.m_payloadSize = 42;
        header->m_info.m_txTimestamp = std::chrono::steady_clock::now();

        auto sample = static_cast<CounterTopic*>(header->payload());
        sample->data = myData;
        myPublisher.sendChunk(header);
    }

    myPublisher.stopOffer();
}

void sendDataWithDynamicPayloadSize()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        uint32_t myChangingPayloadSize = sizeof(CounterTopic);
        auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(myChangingPayloadSize, true));
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }

    myPublisher.stopOffer();
}

void sendDataWithDynamicPayloadSizeAndCustomHeader()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
    myPublisher.offer();

    while (keepRunning)
    {
        uint32_t myChangingPayloadSize = sizeof(CounterTopic);
        auto header = myPublisher.allocateChunkWithHeader(myChangingPayloadSize, true);
        auto sample = static_cast<CounterTopic*>(header->payload());
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }

    myPublisher.stopOffer();
}

void sendLatestChunkToNewSubscribers()
{
    iox::runtime::PoshRuntime::getInstance("/myApplicationName");
    iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});

    auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));
    sample->data = myInitialData;
    myPublisher.sendChunk(sample);
    myPublisher.enableDoDeliverOnSubscription();


    myPublisher.offer();

    while (keepRunning)
    {
        auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }

    myPublisher.stopOffer();
}

int main()
{
    // adjust this line to the example you would like to have
    auto example = sendLatestChunkToNewSubscribers;


    std::thread exampleThread(example);
    std::thread stopMeLater([&] {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        keepRunning.store(false);
    });

    stopMeLater.join();
    exampleThread.join();

    return (EXIT_SUCCESS);
}
