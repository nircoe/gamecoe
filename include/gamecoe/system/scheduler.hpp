#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace gamecoe
{
    namespace systems
    {
        namespace scheduler
        {
            struct node
            {
                std::string name;
                std::vector<std::uint32_t> reads;
                std::vector<std::uint32_t> writes;
            };

            struct hint
            {
                std::string first;
                std::string second;
            };

            enum class diagnostic_kind : std::uint8_t
            {
                cycle_forced,
                hint_overrides_edge,
                hint_unknown_system,
                hint_self_reference,
                hint_contradiction,
            };

            // What systems holds depends on the kind. For cycle_forced it is the forced system first, then the
            // rest of its cycle. For hint_contradiction it is the path from the skipped hint's second to its first.
            struct diagnostic
            {
                std::vector<std::string> systems;
                std::vector<std::uint32_t> components;
                diagnostic_kind kind;
            };

            struct result
            {
                std::vector<std::size_t> order;
                std::vector<diagnostic> diagnostics;
            };

            [[nodiscard]] result build_order(std::span<const node> nodes, std::span<const hint> hints = {});
        } // namespace scheduler
    } // namespace systems
} // namespace gamecoe
