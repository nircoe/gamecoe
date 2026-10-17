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

            // What systems holds depends on the kind:
            //
            // * cycle_forced: the forced system first, then the rest of its cycle.
            // * hint_overrides_edge: the hint's first and second system.
            // * hint_unknown_system: the hint's names that match no system.
            // * hint_self_reference: the one system named on both sides.
            // * hint_contradiction: the path from the skipped hint's second to its first.
            // * components is only used by cycle_forced and hint_overrides_edge.
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
