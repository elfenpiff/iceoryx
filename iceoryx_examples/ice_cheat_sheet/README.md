# IceOryx Cheat Sheet

## Custom header

## Publisher short

The following definition is assumed for the following table.
```cpp
iox::popo::Publisher p ({"MyService", "MyInstance", "MyEvent"});
```

| task                                      | code |
|:-----------------------------------------:|:-----|
|send data                                  | `p.allocateChunk(sizeof(Data)));` |
|                                           | `p.sendChunk(myData);` |
|check for subscribers                      | `p.hasSubscribers();` |
|dismiss chunk without sending              | `p.freeChunk(myData);` |
|dismiss chunk with custom header without sending | `p.freeChunk(myHeader);` |
|send data with custom header               | `p.allocateChunkWithHeader(sizeof(Data));` |
|                                           | `p.sendChunk(myHeader);` |
|send data with dynamic payload size        | `p.allocateChunk(dynamicSize, true)` |
|send data with dynamic payload size and custom header | `p.allocateChunkWithHeader(dynamicSize, true)` |
|offer service                              | `p.offer();` |
|stop offering service                      | `p.stopOffer();` |

## Publisher detailled
- **send data**
  ```cpp
  iox::runtime::PoshRuntime::getInstance("/myApplicationName");
  iox::popo::Publisher myPublisher({"MyService", "MyInstance", "MyEvent"});
  myPublisher.offer();

  while (keepRunning)
  {
      auto sample = static_cast<CounterTopic*>(
          myPublisher.allocateChunk(sizeof(CounterTopic)));
      sample->data = myData;

      myPublisher.sendChunk(sample);
  }

  myPublisher.stopOffer();
  ```

- **send data only when subscribers are present**
   ```cpp
   while (keepRunning)
   {
       auto sample = static_cast<CounterTopic*>(
           myPublisher.allocateChunk(sizeof(CounterTopic)));
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
   ```

 - **send data with custom header**
   ```cpp
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
   ```

 - **send data with dynamic payload size**
   ```cpp
    while (keepRunning)
    {
        uint32_t myChangingPayloadSize = sizeof(CounterTopic);
        auto sample = static_cast<CounterTopic*>(
            myPublisher.allocateChunk(myChangingPayloadSize, true));
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }
   ```

 - **send data with dynamic payload size and custom header**
   ```cpp
    while (keepRunning)
    {
        uint32_t myChangingPayloadSize = sizeof(CounterTopic);
        auto header = myPublisher.allocateChunkWithHeader(myChangingPayloadSize, true);
        auto sample = static_cast<CounterTopic*>(header->payload());
        sample->data = myData;
        myPublisher.sendChunk(sample);
    }
   ```
 
