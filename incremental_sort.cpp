#include "incremental_sort.h"

#include <iostream>

int main() {
    /*
    uint8_t vals[] = {0, 1, 0, 0, 1, 1, 1,  0, 1, 1, 1, 0, 0, 1, 1, 0};
    auto tie = Tie(vals, vals + sizeof(vals) / sizeof(vals[0]));
    auto iter = TieIterator(tie);
    while (iter.next()) {
        std::cout << '[' << iter.range_first << " " << iter.range_last << ")" << std::endl;
    }
    */

    int matrix[][10] = {
            {1,2,1,4,3,3,8,3,2,7},
            {1,1,3,7,9,3,2,3,8,4},
            {1,1,4,3,9,9,3,7,5,1},
            {1,2,2,2,8,6,2,0,8,9},
            {1,3,2,2,3,8,4,1,9,7},
            {1,1,1,2,8,0,3,4,8,7},
            {1,3,2,1,5,9,2,6,5,3},
            {1,2,3,2,1,1,7,0,6,8},
    };

    auto keys = std::vector<size_t>{0, 1, 2};

    auto comparator = [&matrix](size_t i, size_t j, size_t k) -> int {
        //std::cout << "m[" << i << ", " << j << "]=" << matrix[i][j] << " m[" << i << ", " << k << "]=" << matrix[i][k] << std::endl;
        return matrix[j][i] - matrix[k][i];
    };

    auto idxes = std::vector<size_t>();
    int n = sizeof(matrix) / sizeof(matrix[0]);
    idxes.reserve(n);
    for (int i = 0; i < n; i++) {
        idxes.push_back(i);
    }

    inc_sort(idxes, n, keys, comparator);

    for (int i = 0; i < idxes.size(); i++) {
        std::cout << idxes[i] << ": ";
        for (int j = 0; j < 10; j++) {
            std::cout << ' ' << matrix[idxes[i]][j];
        }
        std::cout << std::endl;
    }

    return 0;
}
