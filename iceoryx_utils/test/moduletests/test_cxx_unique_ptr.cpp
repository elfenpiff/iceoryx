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

#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "test.hpp"

#include <iostream>

using namespace ::testing;

struct Position {
    double_t x = 0.0;
    double_t y = 0.0;
    double_t z = 0.0;
};

class UniquePtrTest : public Test {

public:
    UniquePtrTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

};

//TEST_F(UniquePtrTest, CanBeConstructedWithUndefinedBlob)
//{
//    auto deleter = [](Position* const p){delete p;};
//    uint8_t* buf = new uint8_t[sizeof(Position)];
//    auto ptr = iox::cxx::unique_ptr<Position>(buf, deleter);

//    ptr->x = 10.0;
//    ptr->y = 77.77;
//    ptr->z = 50.50;
//    EXPECT_EQ(10.0, ptr->x);
//    EXPECT_EQ(77.77, ptr->y);
//    EXPECT_EQ(50.50, ptr->z);
//}

//TEST_F(UniquePtrTest, CanBeResetToPointToUndefinedBlob)
//{
//    auto deleter = [](Position* const p){delete p;};
//    auto ptr = iox::cxx::unique_ptr<Position>(deleter);

//    uint8_t* buf = new uint8_t[sizeof(Position)];
//    ptr.reset(reinterpret_cast<Position*>(buf));

//    ptr->x = 10.0;
//    ptr->y = 77.77;
//    ptr->z = 50.50;
//    EXPECT_EQ(10.0, ptr->x);
//    EXPECT_EQ(77.77, ptr->y);
//    EXPECT_EQ(50.50, ptr->z);
//}

TEST_F(UniquePtrTest, DeleterIsCalledWhenPtrGoesOutOfScope)
{
    bool deleterCalled = false;
    auto deleter = [&deleterCalled](Position* const p){
        deleterCalled = true;
        delete p;
    };

    {
        auto object = new Position();
        auto ptr = iox::cxx::unique_ptr<Position>(object, deleter);
    }

    ASSERT_EQ(true, deleterCalled);
}

TEST_F(UniquePtrTest, DeleterIsProperlySet)
{

}

TEST_F(UniquePtrTest, DeleterNotCalledOnReleasedPointers)
{

}

TEST_F(UniquePtrTest, DeleterNotCalledOnNullptrs)
{

}

TEST_F(UniquePtrTest, CanResetToNullptr)
{

}

TEST_F(UniquePtrTest, CanResetToAnExistingRawPtr)
{

}
