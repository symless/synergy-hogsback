#include <synergy/service/Screen.h>

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
