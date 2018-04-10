#pragma once

#include <queue>
#include <vector>

template <typename T>
class PriorityJobQueues {
public:
    PriorityJobQueues(int numberOfQueue = 0) :
        m_queues(numberOfQueue) {
    }

    void appendQueue() {
        std::queue<T> q;
        m_queues.emplace_back(std::move(q));
    }

    bool appendJob(typename std::queue<T>::size_type index, T job) {
        bool result = false;
        if (index >= m_queues.size()) {
            return result;
        }

        m_queues[index].emplace(std::move(job));
    }

    bool hasJob() noexcept {
        for (auto& q : m_queues) {
            if (!q.empty()) {
                return true;
            }
        }

        return false;
    }

    T& nextJob() {
        for (auto& q : m_queues) {
            if (!q.empty()) {
                return q.front();
            }
        }

        throw;
    }

    void popJob() {
        for (auto& q : m_queues) {
            if (!q.empty()) {
                return q.pop();
            }
        }
    }

private:
    std::vector<std::queue<T>> m_queues;
};
