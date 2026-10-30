#include <l1/tracker.hpp>

namespace paoo {

int& liveBlocks() { static int n = 0; return n; }
int& copyCount()  { static int n = 0; return n; }
int& moveCount()  { static int n = 0; return n; }

void resetCounters()
{
    liveBlocks() = 0;
    copyCount() = 0;
    moveCount() = 0;
}

} // namespace paoo