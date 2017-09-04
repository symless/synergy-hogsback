#ifndef SYNERGY_COMMON_SCREEN_LINKS_H
#define SYNERGY_COMMON_SCREEN_LINKS_H

#include <synergy/common/Screen.h>
#include <memory>
#include <array>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/icl/interval_map.hpp>

class ScreenLinks;
using ScreenEdge = boost::icl::interval_map<int64_t, ScreenID>;

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
    ScreenLinks();
    ScreenNeighbourIterator begin();
    ScreenNeighbourIterator end();

    auto& edges() { return *m_edges; }
    auto const& edges() const { return *m_edges; }
    std::size_t size() const;
    bool empty() const { return size() == 0; }

    ScreenEdge& top() const { return m_edges->at(0); }
    ScreenEdge& right() const { return m_edges->at(1); }
    ScreenEdge& bottom() const { return m_edges->at(2); }
    ScreenEdge& left() const { return m_edges->at(3); }

private:
    std::unique_ptr<std::array<ScreenEdge, 4>> m_edges;
};


struct ScreenLinkMap final {
    Screen* m_screen = nullptr;
    ScreenLinks* m_links = nullptr;

    ScreenLinkMap (Screen* screen, ScreenLinks* links) noexcept:
        m_screen(screen), m_links(links) {}

    auto& edges() const noexcept { return m_links->edges(); }
    Screen* operator->() const noexcept { return m_screen; }
    operator Screen&() const noexcept { return *m_screen; }
};

#endif
