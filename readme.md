# std::set analog

This project is my implementation of `std::set`. But there are some differences.

1. `std::set` do not invalidates iterator. But I do.
2. `std::set` returns `set.begin()` on `O(1)` time. But I do it on `O(log n)` time.
3. `std::set` based on [red-black tree](https://en.wikipedia.org/wiki/Red–black_tree), while mine on [aa-tree](https://en.wikipedia.org/wiki/AA_tree). They are almost the same, but different in memory usage.
4. I forgot to implement `set.upper_bound()`. My bad.
5. My set can be optimized through [move-opetator](https://en.cppreference.com/w/cpp/language/move_assignment).  
