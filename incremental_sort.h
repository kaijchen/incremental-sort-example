//
// Created by ckj on 5/26/23.
//

#ifndef DORIS_INCREMENTAL_SORT_H
#define DORIS_INCREMENTAL_SORT_H

#include <bits/stdint-uintn.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>
//#include "gutil/integral_types.h"

using Tie = std::vector<uint8_t>;

class TieIterator {
public:
    const Tie& tie;
    const int begin;
    const int end;

    // For outer access
    int range_first;
    int range_last;

    TieIterator(const Tie& tie) : TieIterator(tie, 0, tie.size()) {}

    TieIterator(const Tie& tie, int begin, int end) : tie(tie), begin(begin), end(end) {
        range_first = begin;
        range_last = end;
        _inner_range_first = begin;
        _inner_range_last = end;
    }

    // Iterate the tie
    // Return false means the loop should terminate
    bool next();

private:
    int _inner_range_first;
    int _inner_range_last;

    template <class T>
    static size_t find_byte(const std::vector<T>& list, size_t start, T byte) {
        if (start >= list.size()) {
            return start;
        }
        const void* p = std::memchr((const void*)(list.data() + start), byte, list.size() - start);
        if (p == nullptr) {
            return list.size();
        }
        return (T*)p - list.data();
    }

    // Find position for zero byte, return size of list if not found
    size_t _find_zero(const std::vector<uint8_t>& list, size_t start) {
        return find_byte<uint8_t>(list, start, 0);
    }

    size_t _find_nonzero(const std::vector<uint8_t>& list, size_t start) {
        return find_byte<uint8_t>(list, start, 1);
    }
};

bool TieIterator::next() {
    if (_inner_range_first >= end) {
        return false;
    }

    // Find the first `1`
    if (_inner_range_first == 0 && tie[_inner_range_first] == 1) {
        // Just start from the 0
    } else {
        _inner_range_first = _find_nonzero(tie, _inner_range_first + 1);
        if (_inner_range_first >= end) {
            return false;
        }
        _inner_range_first--;
    }

    // Find the zero, or the end of range
    _inner_range_last = _find_zero(tie, _inner_range_first + 1);
    _inner_range_last = std::min(_inner_range_last, end);

    if (_inner_range_first >= _inner_range_last) {
        return false;
    }

    range_first = _inner_range_first;
    range_last = _inner_range_last;
    _inner_range_first = _inner_range_last;
    return true;
}

void inc_sort_iter(std::vector<size_t>& idxes, Tie& tie, const std::function<int (size_t, size_t)>& cmp) {
    TieIterator iter = TieIterator(tie);
    auto lesser = [&](size_t lhs, size_t rhs) {
        return cmp(lhs, rhs) < 0;
    };
    auto greater = [&](size_t lhs, size_t rhs) { return cmp(lhs, rhs) > 0; };
    while (iter.next()) {
        std::sort(idxes.begin() + iter.range_first, idxes.begin() + iter.range_last, lesser);
        tie[iter.range_first] = 0;
        for (int i = iter.range_first + 1; i < iter.range_last; i++) {
            tie[i] &= cmp(idxes[i - 1], idxes[i]) == 0;
        }
    }
}

void inc_sort(std::vector<size_t>& idxes, size_t num, std::vector<size_t> keys,
              const std::function<int (size_t, size_t, size_t)> comparator) {
    Tie tie = Tie(num, 1);
    for (auto key : keys) {
        auto cmp = [&comparator, &key](size_t i, size_t j) -> int {
           return comparator(key, i, j);
        };
        inc_sort_iter(idxes, tie, cmp);
    }
}

#endif //DORIS_INCREMENTAL_SORT_H
