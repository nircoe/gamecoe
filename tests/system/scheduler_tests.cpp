#include <gtest/gtest.h>
#include <gamecoe/system/scheduler.hpp>
#include <cstddef>
#include <cstdint>
#include <format>
#include <numeric>
#include <source_location>
#include <string>
#include <utility>
#include <vector>

using namespace gamecoe::systems;

namespace
{
    constexpr std::uint32_t comp_x = 0, comp_y = 1, comp_z = 2;

    scheduler::node make_node(std::string name, std::vector<std::uint32_t> reads = {},
        std::vector<std::uint32_t> writes = {})
    {
        return {std::move(name), std::move(reads), std::move(writes)};
    }

    std::vector<std::string> ordered_names(const std::vector<scheduler::node> &nodes, const scheduler::result &result)
    {
        std::vector<std::string> names;
        for (std::size_t index : result.order)
            names.push_back(nodes[index].name);
        return names;
    }

    void expect_diagnostic(const scheduler::result &result, std::size_t index, scheduler::diagnostic_kind kind,
        const std::vector<std::string> &systems, std::source_location where = std::source_location::current())
    {
        SCOPED_TRACE(std::format("called from line {}", where.line()));
        ASSERT_LT(index, result.diagnostics.size());
        EXPECT_EQ(result.diagnostics[index].kind, kind);
        EXPECT_EQ(result.diagnostics[index].systems, systems);
    }
} // namespace

//==============================================================================
//                SchedulerTests - systems::scheduler ordering tests
//==============================================================================

class SchedulerTests : public ::testing::Test
{
};

//==============================================================================
//                         Ordering and cycle tests
//==============================================================================

TEST_F(SchedulerTests, WriterRunsBeforeReaderRegardlessOfRegistrationOrder)
{
    // Test 1: writer registered first
    {
        const std::vector<scheduler::node> nodes{make_node("writer", {}, {comp_x}), make_node("reader", {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"writer", "reader"}));
        EXPECT_TRUE(result.diagnostics.empty());
    }

    // Test 2: reader registered first
    {
        const std::vector<scheduler::node> nodes{make_node("reader", {comp_x}), make_node("writer", {}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 0}));
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"writer", "reader"}));
        EXPECT_TRUE(result.diagnostics.empty());
    }
}

TEST_F(SchedulerTests, TransformCameraRenderOrderFromAccessSets)
{
    constexpr std::uint32_t world_transform = 0, camera_matrices = 1;
    const std::vector<scheduler::node> base{
        make_node("transform_system", {}, {world_transform}),
        make_node("camera_system", {world_transform}, {camera_matrices}),
        make_node("render_system", {world_transform, camera_matrices}),
    };

    std::vector<std::size_t> permutation(base.size());
    std::iota(permutation.begin(), permutation.end(), 0);
    int checked = 0;

    do
    {
        std::vector<scheduler::node> nodes;
        for (std::size_t index : permutation)
            nodes.push_back(base[index]);

        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(ordered_names(nodes, result),
            (std::vector<std::string>{"transform_system", "camera_system", "render_system"}));
        EXPECT_TRUE(result.diagnostics.empty());
        ++checked;
    } while (std::next_permutation(permutation.begin(), permutation.end()));

    EXPECT_EQ(checked, 6);
}

TEST_F(SchedulerTests, WriteWritePairsKeepRegistrationOrder)
{
    // Test 1: two writers of one component, in both registration orders
    {
        const std::vector<scheduler::node> nodes{make_node("a", {}, {comp_x}), make_node("b", {}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"a", "b"}));
        EXPECT_TRUE(result.diagnostics.empty());

        const std::vector<scheduler::node> swapped{make_node("b", {}, {comp_x}), make_node("a", {}, {comp_x})};
        const scheduler::result swapped_result = scheduler::build_order(swapped);
        EXPECT_EQ(ordered_names(swapped, swapped_result), (std::vector<std::string>{"b", "a"}));
        EXPECT_TRUE(swapped_result.diagnostics.empty());
    }

    // Test 2: three writers of one component
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {}, {comp_x}), make_node("b", {}, {comp_x}), make_node("c", {}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
        EXPECT_TRUE(result.diagnostics.empty());
    }
}

TEST_F(SchedulerTests, DisjointAndReadReadPairsKeepRegistrationOrder)
{
    // Test 1: disjoint components
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_x}, {comp_y}), make_node("b", {comp_z})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
        EXPECT_TRUE(result.diagnostics.empty());
    }

    // Test 2: two readers of the same component
    {
        const std::vector<scheduler::node> nodes{make_node("a", {comp_x}), make_node("b", {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
        EXPECT_TRUE(result.diagnostics.empty());
    }

    // Test 3: empty access sets
    {
        const std::vector<scheduler::node> nodes{make_node("a"), make_node("b"), make_node("c")};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
        EXPECT_TRUE(result.diagnostics.empty());
    }
}

TEST_F(SchedulerTests, DirectedEdgeWinsOverWriteWrite)
{
    const std::vector<scheduler::node> nodes{make_node("b", {comp_x}, {comp_y}), make_node("a", {}, {comp_x, comp_y})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 0}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, LowestRegistrationIndexWinsAmongReadySystems)
{
    const std::vector<scheduler::node> nodes{
        make_node("reader", {comp_x}), make_node("one", {comp_y}), make_node("two", {comp_z}),
        make_node("writer", {}, {comp_x})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 2, 3, 0}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, AccessCycleFallsBackToRegistrationOrder)
{
    // Test 1: two-system cycle
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_x}, {comp_y}), make_node("b", {comp_y}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b"});
        EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
    }

    // Test 2: the same pair registered the other way round
    {
        const std::vector<scheduler::node> nodes{
            make_node("b", {comp_y}, {comp_x}), make_node("a", {comp_x}, {comp_y})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"b", "a"});
    }

    // Test 3: three-system cycle is one group
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_x}, {comp_y}), make_node("b", {comp_y}, {comp_z}), make_node("c", {comp_z}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b", "c"});
    }

    // Test 4: a ready system registered after the cycle runs before any forcing
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_x}, {comp_y}), make_node("b", {comp_y}, {comp_x}), make_node("d", {comp_z})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{2, 0, 1}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b"});
    }
}

TEST_F(SchedulerTests, ForcingNeverBreaksAcyclicEdges)
{
    constexpr std::uint32_t wt = 3;
    const std::vector<scheduler::node> nodes{
        make_node("render", {wt, comp_y}), make_node("a", {comp_z}, {comp_y}), make_node("b", {comp_y}, {comp_z}),
        make_node("transform", {}, {wt})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{3, 1, 0, 2}));
    ASSERT_EQ(result.diagnostics.size(), 1u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b"});
}

TEST_F(SchedulerTests, ChainedCyclesForceTheUpstreamGroupFirst)
{
    constexpr std::uint32_t x = 0, y = 1, w = 2, y2 = 3, z2 = 4;
    const std::vector<scheduler::node> nodes{
        make_node("c", {w, y2}, {z2}), make_node("d", {z2}, {y2}), make_node("a", {x}, {y, w}),
        make_node("b", {y}, {x})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{2, 3, 0, 1}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b"});
    expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"c", "d"});
}

TEST_F(SchedulerTests, ForcingKeepsEdgesAmongRemainingGroupMembers)
{
    // Test 1: ring registered out of edge order
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_x}, {comp_y}), make_node("c", {comp_z}, {comp_x}), make_node("b", {comp_y}, {comp_z})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 2, 1}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "c", "b"});
    }

    // Test 2: every pair cyclic forces twice
    {
        const std::vector<scheduler::node> nodes{
            make_node("a", {comp_y, comp_z}, {comp_x}), make_node("b", {comp_x, comp_z}, {comp_y}),
            make_node("c", {comp_x, comp_y}, {comp_z})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
        ASSERT_EQ(result.diagnostics.size(), 2u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b", "c"});
        EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x, comp_y, comp_z}));
        expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"b", "c"});
        EXPECT_EQ(result.diagnostics[1].components, (std::vector<std::uint32_t>{comp_y, comp_z}));
    }
}

TEST_F(SchedulerTests, CycleDiagnosticListsOnlyEdgesInsideTheCycle)
{
    const std::vector<scheduler::node> nodes{
        make_node("a", {comp_x}, {comp_y, comp_z}), make_node("b", {comp_y}, {comp_x}), make_node("c", {comp_z})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
    ASSERT_EQ(result.diagnostics.size(), 1u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b"});
    EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
}

TEST_F(SchedulerTests, PlacedSystemsDoNotLinkLaterCycles)
{
    const std::vector<scheduler::node> nodes{
        make_node("a", {1, 3}, {0}), make_node("b", {1}, {0}), make_node("c", {0, 1}, {3}),
        make_node("d", {0}, {1, 2})};
    const scheduler::result result = scheduler::build_order(nodes);
    EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"a", "b", "d", "c"}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "b", "c", "d"});
    expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"b", "d"});
    EXPECT_EQ(result.diagnostics[1].components, (std::vector<std::uint32_t>{0, 1}));
}

TEST_F(SchedulerTests, EmptyAndSingleInput)
{
    // Test 1: empty input
    {
        const std::vector<scheduler::node> nodes;
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_TRUE(result.order.empty());
        EXPECT_TRUE(result.diagnostics.empty());
    }

    // Test 2: one system
    {
        const std::vector<scheduler::node> nodes{make_node("a", {comp_x}, {comp_x})};
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0}));
        EXPECT_TRUE(result.diagnostics.empty());
    }
}

//==============================================================================
//                                Hint tests
//==============================================================================

TEST_F(SchedulerTests, HintOverridesDerivedEdge)
{
    const std::vector<scheduler::node> nodes{make_node("writer", {}, {comp_x}), make_node("reader", {comp_x})};

    // Test 1: without a hint the writer runs first
    {
        const scheduler::result result = scheduler::build_order(nodes);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"writer", "reader"}));
    }

    // Test 2: an opposing hint flips the order and is reported
    {
        const std::vector<scheduler::hint> hints{{"reader", "writer"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"reader", "writer"}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_overrides_edge, {"reader", "writer"});
        EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x}));
    }

    // Test 3: an agreeing hint is silent
    {
        const std::vector<scheduler::hint> hints{{"writer", "reader"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"writer", "reader"}));
        EXPECT_TRUE(result.diagnostics.empty());
    }
}

TEST_F(SchedulerTests, HintSilencesAccessCycle)
{
    const std::vector<scheduler::node> nodes{make_node("a", {comp_x}, {comp_y}), make_node("b", {comp_y}, {comp_x})};

    // Test 1: hint a before b
    {
        const std::vector<scheduler::hint> hints{{"a", "b"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        EXPECT_EQ(result.diagnostics[0].kind, scheduler::diagnostic_kind::hint_overrides_edge);
    }

    // Test 2: hint b before a
    {
        const std::vector<scheduler::hint> hints{{"b", "a"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 0}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        EXPECT_EQ(result.diagnostics[0].kind, scheduler::diagnostic_kind::hint_overrides_edge);
    }
}

TEST_F(SchedulerTests, HintOnUnrelatedPairSetsOrder)
{
    const std::vector<scheduler::node> nodes{make_node("a", {comp_x}), make_node("b", {comp_y})};
    const std::vector<scheduler::hint> hints{{"b", "a"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 0}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, HintChainOrdersDisjointSystems)
{
    const std::vector<scheduler::node> nodes{make_node("a"), make_node("b"), make_node("c")};
    const std::vector<scheduler::hint> hints{{"c", "b"}, {"b", "a"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"c", "b", "a"}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, UnknownHintNamesAreReportedAndSkipped)
{
    const std::vector<scheduler::node> nodes{make_node("writer", {}, {comp_x}), make_node("reader", {comp_x})};
    const std::vector<std::size_t> expected_order{0, 1};

    // Test 1: unknown first
    {
        const std::vector<scheduler::hint> hints{{"ghost", "writer"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(result.order, expected_order);
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_unknown_system, {"ghost"});
        EXPECT_TRUE(result.diagnostics[0].components.empty());
    }

    // Test 2: unknown second
    {
        const std::vector<scheduler::hint> hints{{"reader", "ghost"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(result.order, expected_order);
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_unknown_system, {"ghost"});
    }

    // Test 3: both unknown give one diagnostic
    {
        const std::vector<scheduler::hint> hints{{"ghost", "phantom"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(result.order, expected_order);
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_unknown_system, {"ghost", "phantom"});
    }
}

TEST_F(SchedulerTests, SelfHintIsReportedAndSkipped)
{
    const std::vector<scheduler::node> nodes{make_node("a"), make_node("b")};
    const std::vector<scheduler::hint> hints{{"a", "a"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
    ASSERT_EQ(result.diagnostics.size(), 1u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_self_reference, {"a"});
    EXPECT_TRUE(result.diagnostics[0].components.empty());
}

TEST_F(SchedulerTests, ContradictoryHintsAreReported)
{
    const std::vector<scheduler::node> nodes{make_node("a"), make_node("b"), make_node("c")};

    // Test 1: the later hint is skipped
    {
        const std::vector<scheduler::hint> hints{{"a", "b"}, {"b", "a"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"a", "b", "c"}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_contradiction, {"a", "b"});
        EXPECT_TRUE(result.diagnostics[0].components.empty());
    }

    // Test 2: reverse call order skips the other hint
    {
        const std::vector<scheduler::hint> hints{{"b", "a"}, {"a", "b"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"b", "a", "c"}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_contradiction, {"b", "a"});
    }

    // Test 3: the hint closing a three-hint cycle is skipped
    {
        const std::vector<scheduler::hint> hints{{"a", "b"}, {"b", "c"}, {"c", "a"}};
        const scheduler::result result = scheduler::build_order(nodes, hints);
        EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"a", "b", "c"}));
        ASSERT_EQ(result.diagnostics.size(), 1u);
        expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_contradiction, {"a", "b", "c"});
    }
}

TEST_F(SchedulerTests, DuplicateHintIsIgnoredSilently)
{
    const std::vector<scheduler::node> nodes{make_node("a"), make_node("b")};
    const std::vector<scheduler::hint> hints{{"a", "b"}, {"a", "b"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, HintForwardReferenceResolves)
{
    const std::vector<scheduler::node> nodes{make_node("early"), make_node("middle"), make_node("late")};
    const std::vector<scheduler::hint> hints{{"late", "early"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"middle", "late", "early"}));
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST_F(SchedulerTests, HintAndDerivedEdgesCanStillCycle)
{
    constexpr std::uint32_t p = 3, q = 4;
    const std::vector<scheduler::node> nodes{
        make_node("a", {comp_y}, {p}), make_node("b", {q}, {comp_x}), make_node("c", {comp_x}, {comp_y})};
    const std::vector<scheduler::hint> hints{{"a", "b"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_overrides_edge, {"a", "b"});
    expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"a", "b", "c"});
    EXPECT_EQ(result.diagnostics[1].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
}

TEST_F(SchedulerTests, AgreeingHintKeepsTheDerivedEdgeComponents)
{
    const std::vector<scheduler::node> nodes{
        make_node("a", {2}, {0}), make_node("b", {0}, {1}), make_node("c", {1}, {2})};
    const std::vector<scheduler::hint> hints{{"a", "b"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{0, 1, 2}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_overrides_edge, {"a", "b"});
    EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{1, 2}));
    expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"a", "b", "c"});
    EXPECT_EQ(result.diagnostics[1].components, (std::vector<std::uint32_t>{0, 1, 2}));
}

TEST_F(SchedulerTests, HintOverDerivedPathIsReported)
{
    const std::vector<scheduler::node> nodes{
        make_node("A", {}, {comp_x}), make_node("B", {comp_x}, {comp_y}), make_node("C", {comp_y})};
    const std::vector<scheduler::hint> hints{{"C", "A"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(ordered_names(nodes, result), (std::vector<std::string>{"B", "C", "A"}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::hint_overrides_edge, {"C", "A"});
    EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
    expect_diagnostic(result, 1, scheduler::diagnostic_kind::cycle_forced, {"B", "A", "C"});
    EXPECT_EQ(result.diagnostics[1].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
}

TEST_F(SchedulerTests, ForcingNeverBreaksAnAcceptedHint)
{
    const std::vector<scheduler::node> nodes{
        make_node("a", {}, {comp_x}), make_node("b", {comp_y}), make_node("c", {comp_x}, {comp_y})};
    const std::vector<scheduler::hint> hints{{"b", "a"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{1, 0, 2}));
    ASSERT_EQ(result.diagnostics.size(), 2u);
    EXPECT_EQ(result.diagnostics[0].kind, scheduler::diagnostic_kind::hint_overrides_edge);
    EXPECT_EQ(result.diagnostics[1].kind, scheduler::diagnostic_kind::cycle_forced);
    EXPECT_EQ(result.diagnostics[1].systems.front(), "b");
}

TEST_F(SchedulerTests, ForcingIgnoresHintedPredecessorsAlreadyPlaced)
{
    const std::vector<scheduler::node> nodes{
        make_node("a", {comp_x}, {comp_y}), make_node("c", {comp_y}, {comp_x}), make_node("p")};
    const std::vector<scheduler::hint> hints{{"p", "a"}};
    const scheduler::result result = scheduler::build_order(nodes, hints);
    EXPECT_EQ(result.order, (std::vector<std::size_t>{2, 0, 1}));
    ASSERT_EQ(result.diagnostics.size(), 1u);
    expect_diagnostic(result, 0, scheduler::diagnostic_kind::cycle_forced, {"a", "c"});
    EXPECT_EQ(result.diagnostics[0].components, (std::vector<std::uint32_t>{comp_x, comp_y}));
}
