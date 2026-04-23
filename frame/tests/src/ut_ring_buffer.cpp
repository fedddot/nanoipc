#include "gtest/gtest.h"
#include <stdexcept>

#include "ring_buffer.hpp"

using namespace nanoipc;

TEST(ut_ring_buffer, constructor_initializes_empty_buffer) {
    // WHEN
    RingBuffer<10> buffer;

    // THEN
    ASSERT_EQ(buffer.size(), 0);
}

TEST(ut_ring_buffer, push_back_and_pop_front) {
    // GIVEN
    RingBuffer<10> buffer;

    // WHEN
    buffer.push_back(42);
    buffer.push_back(100);

    // THEN
    ASSERT_EQ(buffer.size(), 2);
    ASSERT_EQ(buffer.pop_front(), 42);
    ASSERT_EQ(buffer.size(), 1);
    ASSERT_EQ(buffer.pop_front(), 100);
    ASSERT_EQ(buffer.size(), 0);
}

TEST(ut_ring_buffer, pop_front_empty_buffer_returns_zero) {
    // GIVEN
    RingBuffer<10> buffer;

    // WHEN
    const auto value = buffer.pop_front();

    // THEN
    ASSERT_EQ(value, 0);
    ASSERT_EQ(buffer.size(), 0);
}

TEST(ut_ring_buffer, get_retrieves_element_without_removing) {
    // GIVEN
    RingBuffer<10> buffer;
    buffer.push_back(10);
    buffer.push_back(20);
    buffer.push_back(30);

    // WHEN
    const auto first = buffer.get(0);
    const auto second = buffer.get(1);
    const auto third = buffer.get(2);

    // THEN
    ASSERT_EQ(first, 10);
    ASSERT_EQ(second, 20);
    ASSERT_EQ(third, 30);
    ASSERT_EQ(buffer.size(), 3);
}

TEST(ut_ring_buffer, get_throws_on_index_out_of_range) {
    // GIVEN
    RingBuffer<10> buffer;
    buffer.push_back(10);
    buffer.push_back(20);

    // THEN
    ASSERT_THROW(buffer.get(2), std::out_of_range);
    ASSERT_THROW(buffer.get(10), std::out_of_range);
}

TEST(ut_ring_buffer, get_throws_on_empty_buffer) {
    // GIVEN
    RingBuffer<10> buffer;

    // THEN
    ASSERT_THROW(buffer.get(0), std::out_of_range);
}

TEST(ut_ring_buffer, wrap_around_behavior) {
    // GIVEN
    RingBuffer<5> buffer;

    // WHEN
    for (int i = 0; i < 7; ++i) {
        buffer.push_back(i);
    }

    // THEN - buffer should contain last 5 values: 2, 3, 4, 5, 6
    ASSERT_EQ(buffer.size(), 5);
    ASSERT_EQ(buffer.get(0), 2);
    ASSERT_EQ(buffer.get(1), 3);
    ASSERT_EQ(buffer.get(2), 4);
    ASSERT_EQ(buffer.get(3), 5);
    ASSERT_EQ(buffer.get(4), 6);
}

TEST(ut_ring_buffer, get_throws_after_wrapping) {
    // GIVEN
    RingBuffer<5> buffer;
    for (int i = 0; i < 7; ++i) {
        buffer.push_back(i);
    }

    // THEN
    ASSERT_THROW(buffer.get(5), std::out_of_range);
}
