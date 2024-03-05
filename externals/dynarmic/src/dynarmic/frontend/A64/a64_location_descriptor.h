/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <functional>
#include <string>
#include <tuple>

#include <fmt/format.h>
#include <mcl/bit/bit_field.hpp>
#include <mcl/stdint.hpp>

#include "dynarmic/common/fp/fpcr.h"
#include "dynarmic/ir/location_descriptor.h"

namespace Dynarmic::A64 {

/**
 * LocationDescriptor describes the location of a basic block.
 * The location is not solely based on the PC because other flags influence the way
 * instructions should be translated.
 */
class LocationDescriptor {
public:
    static constexpr size_t pc_bit_count = 56;
    static constexpr u64 pc_mask = mcl::bit::ones<u64>(pc_bit_count);
    static constexpr u32 fpcr_mask = 0x07C8'0000;
    static constexpr size_t fpcr_shift = 37;
    static constexpr size_t single_stepping_bit = 57;
    static_assert((pc_mask & (u64(fpcr_mask) << fpcr_shift) & (u64(1) << single_stepping_bit)) == 0);

    LocationDescriptor(u64 pc, FP::FPCR fpcr, bool single_stepping = false)
            : pc(pc & pc_mask), fpcr(fpcr.Value() & fpcr_mask), single_stepping(single_stepping) {}

    explicit LocationDescriptor(const IR::LocationDescriptor& o)
            : pc(o.Value() & pc_mask)
            , fpcr((o.Value() >> fpcr_shift) & fpcr_mask)
            , single_stepping(mcl::bit::get_bit<single_stepping_bit>(o.Value())) {}

    u64 PC() const { return mcl::bit::sign_extend<pc_bit_count>(pc); }
    FP::FPCR FPCR() const { return fpcr; }
    bool SingleStepping() const { return single_stepping; }

    bool operator==(const LocationDescriptor& o) const {
        return std::tie(pc, fpcr, single_stepping) == std::tie(o.pc, o.fpcr, o.single_stepping);
    }

    bool operator!=(const LocationDescriptor& o) const {
        return !operator==(o);
    }

    LocationDescriptor SetPC(u64 new_pc) const {
        return LocationDescriptor(new_pc, fpcr, single_stepping);
    }

    LocationDescriptor AdvancePC(int amount) const {
        return LocationDescriptor(static_cast<u64>(pc + amount), fpcr, single_stepping);
    }

    LocationDescriptor SetSingleStepping(bool new_single_stepping) const {
        return LocationDescriptor(pc, fpcr, new_single_stepping);
    }

    u64 UniqueHash() const noexcept {
        // This value MUST BE UNIQUE.
        // This calculation has to match up with EmitTerminalPopRSBHint
        const u64 fpcr_u64 = static_cast<u64>(fpcr.Value()) << fpcr_shift;
        const u64 single_stepping_u64 = static_cast<u64>(single_stepping) << single_stepping_bit;
        return pc | fpcr_u64 | single_stepping_u64;
    }

    operator IR::LocationDescriptor() const {
        return IR::LocationDescriptor{UniqueHash()};
    }

private:
    u64 pc;         ///< Current program counter value.
    FP::FPCR fpcr;  ///< Floating point control register.
    bool single_stepping;
};

/**
 * Provides a string representation of a LocationDescriptor.
 *
 * @param descriptor The descriptor to get a string representation of
 */
std::string ToString(const LocationDescriptor& descriptor);

}  // namespace Dynarmic::A64

namespace std {
template<>
struct less<Dynarmic::A64::LocationDescriptor> {
    bool operator()(const Dynarmic::A64::LocationDescriptor& x, const Dynarmic::A64::LocationDescriptor& y) const noexcept {
        return x.UniqueHash() < y.UniqueHash();
    }
};
template<>
struct hash<Dynarmic::A64::LocationDescriptor> {
    size_t operator()(const Dynarmic::A64::LocationDescriptor& x) const noexcept {
        return std::hash<u64>()(x.UniqueHash());
    }
};
}  // namespace std

template<>
struct fmt::formatter<Dynarmic::A64::LocationDescriptor> : fmt::formatter<std::string> {
    template<typename FormatContext>
    auto format(Dynarmic::A64::LocationDescriptor descriptor, FormatContext& ctx) const {
        return formatter<std::string>::format(Dynarmic::A64::ToString(descriptor), ctx);
    }
};
