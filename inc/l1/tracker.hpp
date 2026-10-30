#ifndef PAOO_TRACKER_HPP
#define PAOO_TRACKER_HPP

namespace paoo {

///@brief Number of heap blocks currently allocated (should be 0 at the end of a clean program)
int& liveBlocks();

///@brief Number of deep copies / moves performed
int& copyCount();
int& moveCount();

void resetCounters();

} // namespace paoo

#endif // PAOO_TRACKER_HPP