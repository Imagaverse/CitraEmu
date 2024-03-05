/* This file is part of the dynarmic project.
 * Copyright (c) 2019 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

namespace Dynarmic::FP {

class FPCR;
class FPSR;

template<typename FPT>
FPT FPRecipExponent(FPT op, FPCR fpcr, FPSR& fpsr);

}  // namespace Dynarmic::FP
