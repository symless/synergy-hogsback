#include <synergy/common/ScreenLinks.h>

ScreenLinks::ScreenLinks(): m_edges(std::make_unique<std::array<ScreenEdge, 4>>()) {
}

ScreenLinkIterator
ScreenLinks::begin() {
    using std::begin;
    /* FIXME */
    return ScreenLinkIterator(m_edges.get(), begin((*m_edges)[0]), 0);
}

ScreenLinkIterator
ScreenLinks::end() {
    using std::end;
    /* FIXME */
    return ScreenLinkIterator(m_edges.get(), end((*m_edges)[3]), 4);
}

size_t
ScreenLinks::size() const {
    auto size = 0;
    for (auto& edge: edges()) {
        size += edge.iterative_size();
    }
    return size;
}
