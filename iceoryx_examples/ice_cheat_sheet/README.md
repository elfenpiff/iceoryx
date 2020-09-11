# IceOryx Cheat Sheet

## Missing Features
 - untyped publisher custom header? allocateChunkWithHeader
 - subscriber inherit from condition?

## UntypedPublisher short

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
        myPublisher.publish(*sample);
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
    myPublisher.loan(sizeof(CounterTopic))
        .and_then([&](iox::popo::Sample<void>& sample) {
            myPublisher.release(sample);
        });
    ```

 - **acquire previous data and change minor contents**
    ```cpp
    myPublisher.previousSample().and_then([&](iox::popo::Sample<void>& sample) {
        static_cast<CounterTopic*>(sample.get())->hugeData[2] = 42;
        sample.publish();
    });
    ```
