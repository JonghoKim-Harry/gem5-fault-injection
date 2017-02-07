#ifndef __CPU_MINOR_PIPE_STATS_HH__
#define __CPU_MINOR_PIPE_STATS_HH__
#include "base/statistics.hh"

namespace Minor
{

class PipelineStats
{
  public:
    void regStats();
    Stats::Scalar num_snapshot;
};

} // namespace Minor

#endif // __CPU_MINOR_PIPE_STATS_HH__
