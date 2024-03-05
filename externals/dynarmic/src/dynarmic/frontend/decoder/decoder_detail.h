/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <algorithm>
#include <array>
#include <tuple>

#include <mcl/assert.hpp>
#include <mcl/bitsizeof.hpp>
#include <mcl/type_traits/function_info.hpp>

namespace Dynarmic::Decoder {
namespace detail {

template<size_t N>
inline consteval std::array<char, N> StringToArray(const char (&str)[N + 1]) {
    std::array<char, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = str[i];
    }
    return result;
}

/**
 * Helper functions for the decoders.
 *
 * @tparam MatcherT The type of the Matcher to use.
 */
template<class MatcherT>
struct detail {
    using opcode_type = typename MatcherT::opcode_type;
    using visitor_type = typename MatcherT::visitor_type;

    static constexpr size_t opcode_bitsize = mcl::bitsizeof<opcode_type>;

    /**
     * Generates the mask and the expected value after masking from a given bitstring.
     * A '0' in a bitstring indicates that a zero must be present at that bit position.
     * A '1' in a bitstring indicates that a one must be present at that bit position.
     */
#ifdef __clang__
    static constexpr auto GetMaskAndExpect(std::array<char, opcode_bitsize> bitstring) {
#else
    static consteval auto GetMaskAndExpect(std::array<char, opcode_bitsize> bitstring) {
#endif
        const auto one = static_cast<opcode_type>(1);
        opcode_type mask = 0, expect = 0;
        for (size_t i = 0; i < opcode_bitsize; i++) {
            const size_t bit_position = opcode_bitsize - i - 1;
            switch (bitstring[i]) {
            case '0':
                mask |= one << bit_position;
                break;
            case '1':
                expect |= one << bit_position;
                mask |= one << bit_position;
                break;
            default:
                // Ignore
                break;
            }
        }
        return std::make_tuple(mask, expect);
    }

    /**
     * Generates the masks and shifts for each argument.
     * A '-' in a bitstring indicates that we don't care about that value.
     * An argument is specified by a continuous string of the same character.
     */
    template<size_t N>
    static consteval auto GetArgInfo(std::array<char, opcode_bitsize> bitstring) {
        std::array<opcode_type, N> masks = {};
        std::array<size_t, N> shifts = {};
        size_t arg_index = 0;
        char ch = 0;

        for (size_t i = 0; i < opcode_bitsize; i++) {
            if (bitstring[i] == '0' || bitstring[i] == '1' || bitstring[i] == '-') {
                if (ch != 0) {
                    ch = 0;
                    arg_index++;
                }
            } else {
                if (ch == 0) {
                    ch = bitstring[i];
                } else if (ch != bitstring[i]) {
                    ch = bitstring[i];
                    arg_index++;
                }

                if constexpr (N > 0) {
                    const size_t bit_position = opcode_bitsize - i - 1;

                    if (arg_index >= N)
                        throw std::out_of_range("Unexpected field");

                    masks[arg_index] |= static_cast<opcode_type>(1) << bit_position;
                    shifts[arg_index] = bit_position;
                } else {
                    throw std::out_of_range("Unexpected field");
                }
            }
        }

#if !defined(DYNARMIC_IGNORE_ASSERTS) && !defined(__ANDROID__)
        // Avoids a MSVC ICE, and avoids Android NDK issue.
        ASSERT(std::all_of(masks.begin(), masks.end(), [](auto m) { return m != 0; }));
#endif

        return std::make_tuple(masks, shifts);
    }

    /**
     * This struct's Make member function generates a lambda which decodes an instruction based on
     * the provided arg_masks and arg_shifts. The Visitor member function to call is provided as a
     * template argument.
     */
    template<typename FnT>
    struct VisitorCaller;

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)
#endif
    template<typename Visitor, typename... Args, typename CallRetT>
    struct VisitorCaller<CallRetT (Visitor::*)(Args...)> {
        template<size_t... iota>
        static auto Make(std::integer_sequence<size_t, iota...>,
                         CallRetT (Visitor::*const fn)(Args...),
                         const std::array<opcode_type, sizeof...(iota)> arg_masks,
                         const std::array<size_t, sizeof...(iota)> arg_shifts) {
            static_assert(std::is_same_v<visitor_type, Visitor>, "Member function is not from Matcher's Visitor");
            return [fn, arg_masks, arg_shifts](Visitor& v, opcode_type instruction) {
                (void)instruction;
                (void)arg_masks;
                (void)arg_shifts;
                return (v.*fn)(static_cast<Args>((instruction & arg_masks[iota]) >> arg_shifts[iota])...);
            };
        }
    };

    template<typename Visitor, typename... Args, typename CallRetT>
    struct VisitorCaller<CallRetT (Visitor::*)(Args...) const> {
        template<size_t... iota>
        static auto Make(std::integer_sequence<size_t, iota...>,
                         CallRetT (Visitor::*const fn)(Args...) const,
                         const std::array<opcode_type, sizeof...(iota)> arg_masks,
                         const std::array<size_t, sizeof...(iota)> arg_shifts) {
            static_assert(std::is_same_v<visitor_type, const Visitor>, "Member function is not from Matcher's Visitor");
            return [fn, arg_masks, arg_shifts](const Visitor& v, opcode_type instruction) {
                (void)instruction;
                (void)arg_masks;
                (void)arg_shifts;
                return (v.*fn)(static_cast<Args>((instruction & arg_masks[iota]) >> arg_shifts[iota])...);
            };
        }
    };
#ifdef _MSC_VER
#    pragma warning(pop)
#endif

    /**
     * Creates a matcher that can match and parse instructions based on bitstring.
     * See also: GetMaskAndExpect and GetArgInfo for format of bitstring.
     */
    template<auto bitstring, typename FnT>
    static auto GetMatcher(FnT fn, const char* const name) {
        constexpr size_t args_count = mcl::parameter_count_v<FnT>;

        constexpr auto mask = std::get<0>(GetMaskAndExpect(bitstring));
        constexpr auto expect = std::get<1>(GetMaskAndExpect(bitstring));
        constexpr auto arg_masks = std::get<0>(GetArgInfo<args_count>(bitstring));
        constexpr auto arg_shifts = std::get<1>(GetArgInfo<args_count>(bitstring));

        using Iota = std::make_index_sequence<args_count>;

        const auto proxy_fn = VisitorCaller<FnT>::Make(Iota(), fn, arg_masks, arg_shifts);
        return MatcherT(name, mask, expect, proxy_fn);
    }
};

#define DYNARMIC_DECODER_GET_MATCHER(MatcherT, fn, name, bitstring) Decoder::detail::detail<MatcherT<V>>::template GetMatcher<bitstring>(&V::fn, name)

}  // namespace detail
}  // namespace Dynarmic::Decoder
