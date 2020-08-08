#include <iostream>
#include <atomic>

class Barrier {
    private:
        std::atomic<int> n;

    public:
        Barrier() {}
        
        Barrier(int n) : n(n) {}

        void set_t(int in) { n = in; return; }

        void dec()  { n--; }

        void barrier() { while (n != 0); return; }

        void wait() { n--; while (n != 0); return; }
};
