#include <l1/tracker.hpp>

namespace paoo
{

    // Function-local statics are initialized on first call, so these counters are
    // safe to use from any other static object regardless of initialization order
    // across translation units
    int &liveBlocks()
    {
        static int n = 0;
        return n;
    }
    int &copyCount()
    {
        static int n = 0;
        return n;
    }
    int &moveCount()
    {
        static int n = 0;
        return n;
    }

    void resetCounters()
    {
        liveBlocks() = 0;
        copyCount() = 0;
        moveCount() = 0;
    }

} // namespace paoo