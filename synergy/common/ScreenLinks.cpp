#include <synergy/common/ScreenLinks.h>

ScreenLinks::ScreenLinks(): m_edges(std::make_unique<std::array<ScreenEdge, 4>>()) {
}

ScreenNeighbourIterator
ScreenLinks::begin() {
    using std::begin;
    auto const edge = 0;
    return ScreenNeighbourIterator(m_edges.get(), begin((*m_edges)[edge]), edge);
}

ScreenNeighbourIterator
ScreenLinks::end() {
    using std::end;
    auto const edge = 3;
    return ScreenNeighbourIterator(m_edges.get(), end((*m_edges)[edge]), edge);
}

size_t
ScreenLinks::size() const {
    auto size = 0;
    for (auto& edge: edges()) {
        size += edge.iterative_size();
    }
    return size;
}
