# IceOryx Cheat Sheet

## Missing Features
 - untyped publisher custom header? allocateChunkWithHeader
 - subscriber inherit from condition?

## Examples
- **single send, single receive**
- **dynamic publish subscribe (offer/stop offer mix)**
  - **subscriber prints message when service is not available**
  - **only send data when there are subscribers**
- **send huge chunks efficiently**
- **1 publisher, 1 subscriber looks for specific service**
- **n publishers, 1 subscriber looks for specific service**
- **dismiss data without sending**

## [NOT YET IMPLEMENTED] UntypedPublisher short
**Hint** Theses examples show a work in progress. The API for the `publisher` is finished but we still have to integrate the new building blocks into RouDi. Till then the examples will not work.

- **get unique publisher id**
- **send raw memory sample**
- **check offer state**

- **send data**
    ```cpp
    myPublisher.loan(sizeof(CounterTopic))
            .and_then([](iox::popo::Sample<void>& sample) {
                static_cast<CounterTopic*>(sample.get())->data = myData;
                sample.publish();
            })
            .or_else([](const iox::popo::AllocationError& errorValue) {
                // perform error handling
            });
    ```
- **send data old school**
    ```cpp
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
    ```

 - **only send data when there are subscribers**
    ```cpp
    if (myPublisher.hasSubscribers())
    {
        myPublisher.loan(sizeof(CounterTopic))
            .and_then([](iox::popo::Sample<void>& sample) {
                static_cast<CounterTopic*>(sample.get())->data = myData;
                sample.publish();
            });
    }
    ```
 
 - **dismiss data without sending**
    ```cpp
    myPublisher.loan(sizeof(CounterTopic));
    ```

 - **acquire previous data and change minor contents**
    ```cpp
    myPublisher.loanPreviousSample().and_then([&](iox::popo::Sample<void>& sample) {
        static_cast<CounterTopic*>(sample.get())->hugeData[2] = 42;
        sample.publish();
    });
    ```

## [NOT YET IMPLEMENTED] UntypedSubscriber short
**Hint** Theses examples show a work in progress. The API for the `subscriber` is finished but we still have to integrate the new building blocks into RouDi. Till then the examples will not work.

- **receive data**
    ```cpp
    mySubscriber.receive()
        .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& maybeSample) {
            maybeSample.and_then([](iox::popo::Sample<const void>& sample) {
                std::cout << "Receiving: " << static_cast<const CounterTopic*>(sample.get())->data << std::endl;
            });
        })
        .or_else([](const iox::popo::ChunkReceiveError& errorValue) {
            // perform error handling
        });
    ```

- **receive data old school**
    ```cpp
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
    ```

- **check subscription state**
    ```cpp
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
    ```