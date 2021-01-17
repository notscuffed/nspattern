/* -------------------------------------------------------------------------------
 * Copyright (c) 2020 notscuffed
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -------------------------------------------------------------------------------
 */

#pragma once

#include <string_view>
#include <array>
#include <vector>
#include <stdexcept>
#include <algorithm>

#define COMPILE_PATTERN(P) ns::pattern_internal::compile_pattern<ns::pattern_internal::count_hex_numbers(std::string_view(P))>(std::string_view(P));

namespace ns
{
    using match_index = std::size_t;
    constexpr match_index no_match = std::numeric_limits<match_index>::max();

    template<class TValue>
    struct pattern_element
    {
        using element_type = TValue;

        TValue value;
        bool any;

        bool operator==(TValue rhs) const
        {
            return any || value == rhs;
        }

        friend bool operator==(TValue lhs, const pattern_element<TValue>& rhs)
        {
            return rhs.any || rhs.value == lhs;
        }
    };

    template<class TValue>
    struct pattern_element_mask
    {
        using element_type = TValue;

        TValue value;
        unsigned char mask;

        bool operator==(TValue rhs) const
        {
            return (rhs & mask) == (value & mask);
        }

        friend bool operator==(TValue lhs, const pattern_element_mask<TValue>& rhs)
        {
            return (lhs & rhs.mask) == (rhs.value & rhs.mask);
        }
    };

    namespace pattern_internal
    {
        constexpr unsigned char hex_digit(const char c)
        {
            if ('0' <= c && c <= '9')
                return c - '0';
            else if ('A' <= c && c <= 'F')
                return c - 'A' + 10;
            else if ('a' <= c && c <= 'f')
                return c - 'a' + 10;
            else
                throw std::domain_error{ "invalid character" };
        }

        constexpr unsigned char parse_hex(std::string_view text)
        {
            if (text.length() != 2)
                throw std::domain_error{ "invalid hex" };

            return (hex_digit(text[0]) << 4) | hex_digit(text[1]);
        }

        constexpr std::size_t count_hex_numbers(std::string_view s)
        {
            return std::count(s.begin(), s.end(), ' ') + 1;
        }

        template <std::size_t N>
        constexpr std::array<pattern_element<unsigned char>, N> compile_pattern(std::string_view s)
        {
            std::array<pattern_element<unsigned char>, N> elements{};
            std::size_t begin = 0;
            std::size_t end = 0;

            for (std::size_t i = 0; i < N && end != std::string_view::npos; i++) {
                end = std::string_view::npos;

                for (std::size_t j = begin; j < s.length(); j++) {
                    if (s[j] == ' ') {
                        end = j;
                        break;
                    }
                }

                if (s[begin] != '?')
                    elements[i] = { parse_hex(s.substr(begin, end - begin)) };
                else
                    elements[i] = { .any = true };

                begin = end + 1;
            }

            return elements;
        }
    }

    std::vector<pattern_element<unsigned char>> compile_pattern(std::string_view s)
    {
        std::vector<pattern_element<unsigned char>> elements;
        const char* begin = s.data();
        const char* input_end = s.data() + s.size();

        while (begin < input_end)
        {
            const char* end = begin;
            while (end < input_end && *end != ' ')
                end++;
            
            if (*begin != '?')
                elements.push_back({ pattern_internal::parse_hex(std::string_view(begin, end - begin)) });
            else
                elements.push_back({ .any = true });

            begin = end + 1;
        }

        return elements;
    }

    template<
        typename TPatternContainer,
        typename TContainer,
        typename = std::enable_if_t<
            std::is_same_v<
                typename TPatternContainer::value_type::element_type,
                std::remove_cv_t<std::remove_pointer_t<decltype(std::declval<TContainer>().data())>>
            >
        >,
        typename = std::enable_if_t<
            std::is_integral_v<decltype(std::declval<TContainer>().size())>
        >
    >
    ns::match_index find_pattern(TPatternContainer const& pattern, TContainer& container)
    {
        auto resultIt = std::search(
            container.data(), container.data() + container.size(),
            pattern.begin(), pattern.end()
        );

        if (resultIt != container.data() + container.size())
            return resultIt - container.data();

        return ns::no_match;
    }

    /// <summary>
    /// Container needs to have .rbegin() and .rend()
    /// </summary>
    template<
        typename TPatternContainer,
        typename TContainer,
        typename = std::enable_if_t<
            std::is_same_v<
                typename TPatternContainer::value_type::element_type,
                std::remove_cv_t<std::remove_pointer_t<decltype(std::declval<TContainer>().data())>>
            >
        >,
        typename = std::enable_if_t<
            std::is_integral_v<decltype(std::declval<TContainer>().size())>
        >,
        typename = decltype(std::declval<TContainer>().rbegin()),
        typename = decltype(std::declval<TContainer>().rend())
    >
    ns::match_index find_pattern_reverse(TPatternContainer const& pattern, TContainer& container)
    {
        auto resultIt = std::search(
            container.rbegin(), container.rend(),
            pattern.rbegin(), pattern.rend()
        );

        if (resultIt != container.rend())
            return std::addressof(*resultIt) - container.data() + 1 - pattern.size();

        return ns::no_match;
    }

    template<
        std::size_t ASize,
        typename TPatternContainer,
        typename TArrayElement,
        typename = std::enable_if_t<
            std::is_same_v<
                typename TPatternContainer::value_type::element_type,
                std::remove_cv_t<TArrayElement>
            >
        >
    >
    ns::match_index find_pattern(TPatternContainer const& pattern, TArrayElement(&array)[ASize])
    {
        auto resultIt = std::search(
            array, array + ASize,
            pattern.begin(), pattern.end()
        );

        if (resultIt != array + ASize)
            return resultIt - array;

        return ns::no_match;
    }
}
