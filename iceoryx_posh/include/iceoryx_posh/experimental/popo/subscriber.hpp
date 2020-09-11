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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{

using uid_t = uint64_t;

enum class SubscriptionState
{
    SUBSCRIBED,
    NOT_SUBSCRIBED,
    SUBSCRIPTION_PENDING
};

enum class SubscriberError : uint8_t
{
    UNKNOWN
};

// ======================================== Base Subscriber ======================================== //

template<typename T, typename port_t = iox::popo::SubscriberPortUser>
class BaseSubscriber : public Condition
{
public:

    BaseSubscriber(const BaseSubscriber& other) = delete;
    BaseSubscriber& operator=(const BaseSubscriber&) = delete;
    BaseSubscriber(BaseSubscriber&& rhs) = delete;
    BaseSubscriber& operator=(BaseSubscriber&& rhs) = delete;
    ~BaseSubscriber() = default;

    ///
    /// @brief uid Get the unique ID of the subscriber.
    /// @return The subscriber's unique ID.
    ///
    uid_t uid() const noexcept;

    ///
    /// @brief getServiceDescription Get the service description of the subscriber.
    /// @return The service description.
    ///
    capro::ServiceDescription getServiceDescription() const noexcept;

    ///
    /// @brief subscribe Initiate subscription.
    /// @param cacheSize
    /// @return
    ///
    cxx::expected<SubscriberError> subscribe(const uint64_t queueCapacity = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;

    ///
    /// @brief getSubscriptionState Get current subscription state.
    /// @return The current subscription state.
    ///
    SubscribeState getSubscriptionState() const noexcept;

    ///
    /// @brief unsubscribe Unsubscribes if currently subscribed, otherwise do nothing.
    ///
    void unsubscribe() noexcept;

    ///
    /// @brief hasData Check if sample is available.
    /// @return True if a new sample is available.
    ///
    bool hasData() noexcept;

    ///
    /// @brief receive Receive the next sample if available.
    /// @return
    /// @details Sample is automatically released when it goes out of scope.
    ///
    cxx::optional<cxx::unique_ptr<T>> receive() noexcept;

    ///
    /// @brief receiveWithHeader Receive the next sample including it's memory chunk header.
    /// @return
    /// @details Sample is automatically released when it goes out of scope.
    ///
    cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>> receiveWithHeader() noexcept;

    ///
    /// @brief clearReceiveBuffer Releases all unread items in the receive buffer.
    ///
    void clearReceiveBuffer() noexcept;

    // Condition overrides
    virtual bool setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept override;
    virtual bool unsetConditionVariable() noexcept override;
    virtual bool hasTriggered() const noexcept override;

protected:
    BaseSubscriber(const capro::ServiceDescription& service);

protected:
    bool m_subscriptionRequested = false;

private:

    port_t m_port{nullptr};

};

// ======================================== Typed Subscriber ======================================== //

template<typename T>
class TypedSubscriber : protected BaseSubscriber<T>
{
public:

    TypedSubscriber(const capro::ServiceDescription& service);
    TypedSubscriber(const TypedSubscriber& other) = delete;
    TypedSubscriber& operator=(const TypedSubscriber&) = delete;
    TypedSubscriber(TypedSubscriber&& rhs) = delete;
    TypedSubscriber& operator=(TypedSubscriber&& rhs) = delete;
    ~TypedSubscriber() = default;

    capro::ServiceDescription getServiceDescription() const noexcept;
    uid_t uid() const noexcept;

    cxx::expected<SubscriberError> subscribe(const uint64_t queueCapacity = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;
    SubscriptionState getSubscriptionState() const noexcept;
    void unsubscribe() noexcept;

    bool hasData() const noexcept;
    cxx::optional<cxx::unique_ptr<T>> receive() noexcept;
    cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>> receiveWithHeader() noexcept;
    void clearReceiveBuffer() noexcept;

};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/subscriber.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP
