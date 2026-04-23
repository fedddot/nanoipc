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

TEST(ut_ring_buffer, pop_front_empty_buffer_throws) {
    // GIVEN
    RingBuffer<10> buffer;

    // THEN
    ASSERT_THROW(buffer.pop_front(), std::out_of_range);
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

TEST(ut_ring_buffer, push_back_throws_on_full_buffer) {
    // GIVEN
    RingBuffer<3> buffer;
    buffer.push_back(1);
    buffer.push_back(2);
    buffer.push_back(3);

    // WHEN / THEN
    ASSERT_THROW(buffer.push_back(4), std::overflow_error);
    ASSERT_EQ(buffer.size(), 3);
}

TEST(ut_ring_buffer, wrap_around_with_pop_and_push) {
    // GIVEN
    RingBuffer<5> buffer;
    buffer.push_back(1);
    buffer.push_back(2);
    buffer.push_back(3);
    buffer.push_back(4);
    buffer.push_back(5);

    // WHEN
    buffer.pop_front();
    buffer.pop_front();
    buffer.push_back(6);
    buffer.push_back(7);

    // THEN
    ASSERT_EQ(buffer.size(), 5);
    ASSERT_EQ(buffer.get(0), 3);
    ASSERT_EQ(buffer.get(1), 4);
    ASSERT_EQ(buffer.get(2), 5);
    ASSERT_EQ(buffer.get(3), 6);
    ASSERT_EQ(buffer.get(4), 7);
}

TEST(ut_ring_buffer, get_throws_after_full_buffer_reached) {
    // GIVEN
    RingBuffer<5> buffer;
    for (int i = 1; i <= 5; ++i) {
        buffer.push_back(i);
    }

    // THEN
    ASSERT_THROW(buffer.get(5), std::out_of_range);
}

