/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <string>

#include <fmt/format.h>
#include <mcl/stdint.hpp>

namespace Dynarmic::IR {

enum class Type;

/**
 * The Opcodes of our intermediate representation.
 * Type signatures for each opcode can be found in opcodes.inc
 */
enum class Opcode {
#define OPCODE(name, type, ...) name,
#define A32OPC(name, type, ...) A32##name,
#define A64OPC(name, type, ...) A64##name,
#include "./opcodes.inc"
#undef OPCODE
#undef A32OPC
#undef A64OPC
    NUM_OPCODE
};

constexpr size_t OpcodeCount = static_cast<size_t>(Opcode::NUM_OPCODE);

/// Get return type of an opcode
Type GetTypeOf(Opcode op);

/// Get the number of arguments an opcode accepts
size_t GetNumArgsOf(Opcode op);

/// Get the required type of an argument of an opcode
Type GetArgTypeOf(Opcode op, size_t arg_index);

/// Get the name of an opcode.
std::string GetNameOf(Opcode op);

}  // namespace Dynarmic::IR

template<>
struct fmt::formatter<Dynarmic::IR::Opcode> : fmt::formatter<std::string> {
    template<typename FormatContext>
    auto format(Dynarmic::IR::Opcode op, FormatContext& ctx) const {
        return formatter<std::string>::format(Dynarmic::IR::GetNameOf(op), ctx);
    }
};
