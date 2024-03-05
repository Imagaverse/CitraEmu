/* This file is part of the dynarmic project.
 * Copyright (c) 2022 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#include "dynarmic/interface/exclusive_monitor.h"

#include <algorithm>

#include <mcl/assert.hpp>

namespace Dynarmic {

ExclusiveMonitor::ExclusiveMonitor(size_t processor_count)
        : exclusive_addresses(processor_count, INVALID_EXCLUSIVE_ADDRESS), exclusive_values(processor_count) {}

size_t ExclusiveMonitor::GetProcessorCount() const {
    return exclusive_addresses.size();
}

void ExclusiveMonitor::Lock() {
    lock.Lock();
}

void ExclusiveMonitor::Unlock() {
    lock.Unlock();
}

bool ExclusiveMonitor::CheckAndClear(size_t processor_id, VAddr address) {
    const VAddr masked_address = address & RESERVATION_GRANULE_MASK;

    Lock();
    if (exclusive_addresses[processor_id] != masked_address) {
        Unlock();
        return false;
    }

    for (VAddr& other_address : exclusive_addresses) {
        if (other_address == masked_address) {
            other_address = INVALID_EXCLUSIVE_ADDRESS;
        }
    }
    return true;
}

void ExclusiveMonitor::Clear() {
    Lock();
    std::fill(exclusive_addresses.begin(), exclusive_addresses.end(), INVALID_EXCLUSIVE_ADDRESS);
    Unlock();
}

void ExclusiveMonitor::ClearProcessor(size_t processor_id) {
    Lock();
    exclusive_addresses[processor_id] = INVALID_EXCLUSIVE_ADDRESS;
    Unlock();
}

}  // namespace Dynarmic
