#include <gamecoe/system/scheduler.hpp>
#include <algorithm>
#include <iterator>
#include <optional>
#include <utility>

namespace gamecoe
{
    namespace systems
    {
        namespace scheduler
        {
            namespace
            {
                struct edge
                {
                    bool present = false;
                    std::vector<std::uint32_t> components;
                };

                std::vector<std::uint32_t> intersect(const std::vector<std::uint32_t> &a,
                    const std::vector<std::uint32_t> &b)
                {
                    std::vector<std::uint32_t> left = a;
                    std::vector<std::uint32_t> right = b;
                    std::ranges::sort(left);
                    std::ranges::sort(right);

                    std::vector<std::uint32_t> common;
                    std::set_intersection(left.begin(), left.end(), right.begin(), right.end(),
                        std::back_inserter(common));
                    const auto duplicates = std::ranges::unique(common);
                    common.erase(duplicates.begin(), duplicates.end());
                    return common;
                }

                // edges[from * n + to] is set when `from` must run before `to`.
                std::vector<edge> derive_edges(std::span<const node> nodes)
                {
                    const std::size_t n = nodes.size();
                    std::vector<edge> edges(n * n);

                    for (std::size_t i = 0; i < n; ++i)
                    {
                        for (std::size_t j = i + 1; j < n; ++j)
                        {
                            std::vector<std::uint32_t> forward = intersect(nodes[i].writes, nodes[j].reads);
                            std::vector<std::uint32_t> backward = intersect(nodes[j].writes, nodes[i].reads);

                            if (forward.empty() && backward.empty())
                            {
                                // No dependency between `from` and `to`, but both write the same thing.
                                // Keep registration order.
                                std::vector<std::uint32_t> ww = intersect(nodes[i].writes, nodes[j].writes);
                                if (!ww.empty())
                                    edges[i * n + j] = {true, std::move(ww)};
                                continue;
                            }

                            if (!forward.empty())
                                edges[i * n + j] = {true, std::move(forward)};
                            if (!backward.empty())
                                edges[j * n + i] = {true, std::move(backward)};
                        }
                    }

                    return edges;
                }

                std::optional<std::size_t> find_index(std::span<const node> nodes, const std::string &name)
                {
                    const auto it = std::ranges::find_if(nodes, [&name](const node &n) { return n.name == name; });
                    if (it == nodes.end())
                        return std::nullopt;
                    return static_cast<std::size_t>(std::distance(nodes.begin(), it));
                }

                std::optional<std::vector<std::size_t>> find_hint_path(const std::vector<bool> &hint_edge,
                    std::size_t n, std::size_t from, std::size_t to)
                {
                    std::vector<std::size_t> predecessor(n, n);
                    std::vector<bool> visited(n, false);
                    std::vector<std::size_t> queue{from};
                    visited[from] = true;

                    for (std::size_t head = 0; head < queue.size(); ++head)
                    {
                        const std::size_t current = queue[head];
                        for (std::size_t next = 0; next < n; ++next)
                        {
                            if (visited[next] || !hint_edge[current * n + next])
                                continue;
                            visited[next] = true;
                            predecessor[next] = current;
                            queue.push_back(next);
                        }
                    }

                    if (!visited[to])
                        return std::nullopt;

                    std::vector<std::size_t> path{to};
                    while (path.back() != from)
                        path.push_back(predecessor[path.back()]);
                    std::ranges::reverse(path);
                    return path;
                }

                std::vector<bool> apply_hints(std::span<const node> nodes, std::span<const hint> hints,
                    std::vector<edge> &edges, std::vector<diagnostic> &diagnostics)
                {
                    const std::size_t n = nodes.size();
                    std::vector<bool> hint_edge(n * n, false);

                    for (const hint &h : hints)
                    {
                        const std::optional<std::size_t> first = find_index(nodes, h.first);
                        const std::optional<std::size_t> second = find_index(nodes, h.second);

                        if (!first || !second)
                        {
                            diagnostic unknown{diagnostic_kind::hint_unknown_system, {}, {}};
                            if (!first)
                                unknown.systems.push_back(h.first);
                            if (!second)
                                unknown.systems.push_back(h.second);
                            diagnostics.push_back(std::move(unknown));
                            continue;
                        }

                        const std::size_t f = *first;
                        const std::size_t s = *second;

                        if (f == s)
                        {
                            diagnostics.push_back({diagnostic_kind::hint_self_reference, {nodes[f].name}, {}});
                            continue;
                        }

                        if (hint_edge[f * n + s])
                            continue;

                        // A path from `second` back to `first` means this hint would make a loop of hints.
                        if (const auto path = find_hint_path(hint_edge, n, s, f))
                        {
                            diagnostic contradiction{diagnostic_kind::hint_contradiction, {}, {}};
                            for (std::size_t index : *path)
                                contradiction.systems.push_back(nodes[index].name);
                            diagnostics.push_back(std::move(contradiction));
                            continue;
                        }

                        hint_edge[f * n + s] = true;
                        if (edges[s * n + f].present)
                            diagnostics.push_back({diagnostic_kind::hint_overrides_edge,
                                {nodes[f].name, nodes[s].name}, edges[s * n + f].components});
                        edges[s * n + f] = {};
                        edges[f * n + s] = {true, {}};
                    }

                    return hint_edge;
                }

                // reach[i * n + j] is true when `j` can be reached from `i` through systems not placed yet.
                std::vector<bool> reachability(const std::vector<edge> &edges, const std::vector<bool> &placed)
                {
                    const std::size_t n = placed.size();
                    std::vector<bool> reach(n * n, false);

                    for (std::size_t i = 0; i < n; ++i)
                        for (std::size_t j = 0; j < n; ++j)
                            reach[i * n + j] = !placed[i] && !placed[j] && edges[i * n + j].present;

                    for (std::size_t k = 0; k < n; ++k)
                    {
                        if (placed[k])
                            continue;
                        for (std::size_t i = 0; i < n; ++i)
                        {
                            if (!reach[i * n + k])
                                continue;
                            for (std::size_t j = 0; j < n; ++j)
                                if (reach[k * n + j])
                                    reach[i * n + j] = true;
                        }
                    }

                    return reach;
                }

                // Only a system that nothing outside its cycle feeds into can be forced.
                // Never one that is waiting on a hint.
                bool is_candidate(std::size_t f, const std::vector<bool> &reach, const std::vector<bool> &hint_edge,
                    const std::vector<bool> &placed)
                {
                    const std::size_t n = placed.size();
                    for (std::size_t p = 0; p < n; ++p)
                    {
                        if (placed[p])
                            continue;
                        if (reach[p * n + f] && !reach[f * n + p])
                            return false;
                        if (hint_edge[p * n + f])
                            return false;
                    }
                    return true;
                }

                std::size_t force_cycle(std::span<const node> nodes, const std::vector<edge> &edges,
                    const std::vector<bool> &hint_edge, const std::vector<bool> &placed,
                    std::vector<diagnostic> &diagnostics)
                {
                    const std::size_t n = nodes.size();
                    const std::vector<bool> reach = reachability(edges, placed);

                    std::size_t f = 0;
                    while (placed[f] || !is_candidate(f, reach, hint_edge, placed))
                        ++f;

                    std::vector<std::size_t> group{f};
                    for (std::size_t p = 0; p < n; ++p)
                        if (p != f && !placed[p] && reach[f * n + p] && reach[p * n + f])
                            group.push_back(p);

                    diagnostic forced{diagnostic_kind::cycle_forced, {}, {}};
                    for (std::size_t from : group)
                    {
                        forced.systems.push_back(nodes[from].name);
                        for (std::size_t to : group)
                        {
                            const std::vector<std::uint32_t> &ids = edges[from * n + to].components;
                            forced.components.insert(forced.components.end(), ids.begin(), ids.end());
                        }
                    }
                    std::ranges::sort(forced.components);
                    const auto duplicates = std::ranges::unique(forced.components);
                    forced.components.erase(duplicates.begin(), duplicates.end());

                    diagnostics.push_back(std::move(forced));
                    return f;
                }

                bool is_blocked(std::size_t i, const std::vector<edge> &edges, const std::vector<bool> &placed)
                {
                    const std::size_t n = placed.size();
                    for (std::size_t p = 0; p < n; ++p)
                        if (p != i && !placed[p] && edges[p * n + i].present)
                            return true;
                    return false;
                }
            } // namespace

            result build_order(std::span<const node> nodes, std::span<const hint> hints)
            {
                const std::size_t n = nodes.size();
                result out;
                std::vector<edge> edges = derive_edges(nodes);
                const std::vector<bool> hint_edge = apply_hints(nodes, hints, edges, out.diagnostics);

                std::vector<bool> placed(n, false);
                out.order.reserve(n);

                while (out.order.size() < n)
                {
                    std::size_t next = 0;
                    while (next < n && (placed[next] || is_blocked(next, edges, placed)))
                        ++next;

                    // Nothing is ready, so what is left is stuck on a cycle. Force one system to break it.
                    if (next == n)
                        next = force_cycle(nodes, edges, hint_edge, placed, out.diagnostics);

                    placed[next] = true;
                    out.order.push_back(next);
                }

                return out;
            }
        } // namespace scheduler
    } // namespace systems
} // namespace gamecoe
