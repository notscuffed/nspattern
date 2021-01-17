#include <span>
#include <vector>
#include <assert.h>

#include "ns/pattern.h"

void test_single_char()
{
    const unsigned char data[] = { 0x00, 0x00, 0x01, 0x6A, 0x03, 0x04, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("6A");

    assert(ns::find_pattern(pattern, data) == 3);
}

void test_many_char()
{
    const unsigned char data[] = { 0x00, 0x00, 0x01, 0x66, 0x03, 0x04, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("01 66 03");

    assert(ns::find_pattern(pattern, data) == 2);
}

void test_find_in_vector()
{
    std::vector<unsigned char> data = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x90, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("90 90");

    assert(ns::find_pattern(pattern, data) == 5);
}

void test_no_match()
{
    std::vector<unsigned char> data = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x90, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("12 34");

    assert(ns::find_pattern(pattern, data) == ns::no_match);
}

void test_span()
{
    const unsigned char data[] = { 0x00, 0x00, 0x01, 0x66, 0x03, 0x04, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("66");

    assert(ns::find_pattern(pattern, std::span(data, sizeof(data))) == 3);
}

void test_mask()
{
    const unsigned char data[] = { 0x00, 0x00, 0x00, 0x01, 0xCA, 0x03, 0x00 };
    constexpr std::array<ns::pattern_element_mask<unsigned char>, 3> pattern = { {
        {0x01, 0xFF},
        {0xFF, 0x0A},
        {0x03, 0xFF}
    } };

    assert(ns::find_pattern(pattern, data) == 3);
}

void test_mask_no_match()
{
    const unsigned char data[] = { 0x00, 0x00, 0x00, 0x01, 0xCC, 0x03, 0x00 };
    constexpr std::array<ns::pattern_element_mask<unsigned char>, 3> pattern = { {
        {0x01, 0xFF},
        {0xFF, 0x0A},
        {0x03, 0xFF}
    } };

    assert(ns::find_pattern(pattern, data) == ns::no_match);
}

struct CustomContainer
{
    std::vector<unsigned char> storage;

    unsigned char* data()
    {
        return storage.data();
    }

    std::size_t size()
    {
        return storage.size();
    }
};

void test_custom_container()
{
    CustomContainer container = { {0xCA, 0xFE, 0xCA, 0xFE, 0x00, 0x90, 0x90} };
    constexpr auto pattern = COMPILE_PATTERN("CA FE");

    assert(ns::find_pattern(pattern, container) == 0);
}

void test_reverse_search()
{
    std::vector<unsigned char> data = { 0x00, 0xAA, 0xAB, 0xCD, 0xEF, 0x90, 0x90 };
    constexpr auto pattern = COMPILE_PATTERN("AA AB ?? EF");

    auto result = ns::find_pattern_reverse(pattern, data);
    assert(result == 1);
}

void test_non_constexpr_reverse_search()
{
    std::vector<unsigned char> data = { 0x00, 0xAA, 0xAB, 0xCD, 0xEF, 0x90, 0x90 };
    auto pattern = ns::compile_pattern("AA AB ?? EF");

    auto result = ns::find_pattern_reverse(pattern, data);
    assert(result == 1);
}

void test_non_constexpr_default()
{
    std::vector<unsigned char> data = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x90, 0x90 };
    auto pattern = ns::compile_pattern("90 90");

    assert(ns::find_pattern(pattern, data) == 5);
}

int main()
{
    test_single_char();
    test_many_char();
    test_find_in_vector();
    test_no_match();
    test_span();
    test_mask();
    test_mask_no_match();
    test_custom_container();
    test_reverse_search();

    test_non_constexpr_reverse_search();
    test_non_constexpr_default();

    return 0;
}
