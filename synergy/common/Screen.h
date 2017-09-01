#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <string>
#include <boost/icl/interval_map.hpp>
#include <cassert>

using ScreenID = int64_t;
using ScreenEdge = boost::icl::interval_map<int64_t, ScreenID>;

class Screen;
class ScreenLinks;

class ScreenNeighbourIterator
    : public boost::iterator_facade<ScreenNeighbourIterator,
                                    ScreenEdge::value_type,
                                    boost::iterators::forward_traversal_tag> {
    friend class Screen;
    friend class ScreenLinks;
    friend class boost::iterators::iterator_core_access;
    using edges_type = std::array<ScreenEdge, 4>;

    edges_type* m_edges;
    ScreenEdge::iterator m_linkItr;
    int m_edgeIndex;

    ScreenNeighbourIterator (std::array<ScreenEdge, 4>* const edges,
                             ScreenEdge::iterator const it,
                             int const edge) noexcept
        : m_edges (edges), m_linkItr (it), m_edgeIndex (edge) {
    }

    decltype (auto)
    dereference() const noexcept {
        return *m_linkItr;
    }

    void
    increment() noexcept {
        using std::begin;
        using std::end;
        assert (m_edgeIndex >= 0);
        if (m_linkItr != end ((*m_edges)[m_edgeIndex])) {
            ++m_linkItr;
            return;
        }
        if (m_edgeIndex >= 3) {
            return;
        }
        ++m_edgeIndex;
        m_linkItr = begin ((*m_edges)[m_edgeIndex]);
    }

    bool
    equal(ScreenNeighbourIterator const& other) const noexcept {
        return (other.m_edges == m_edges) && (other.m_linkItr == m_linkItr) &&
               (other.m_edgeIndex == m_edgeIndex);
    }
};


class ScreenLinks final {
    friend class Screen;
public:
    ScreenNeighbourIterator begin();
    ScreenNeighbourIterator end();

    auto& edges() { return *m_edges; }
    auto const& edges() const { return *m_edges; }

private:
    std::unique_ptr<std::array<ScreenEdge, 4>> m_edges;
};


class Screen final {
public:
    ScreenID    id = 0;
    std::string name;
    int64_t     x = 0;
    int64_t     y = 0;
    int64_t     width = 0;
    int64_t     height = 0;
};


class Config final {
private:
    std::vector<Screen> m_screens;
    std::vector<ScreenLinks> m_links;
};
