#include "cpu/minor/pipeline_stats.hh"

void
Minor::PipelineStats::regStats()
{
    num_snapshot.name("num_snapshot")
                .desc("Number of snapshots")
                ;
}
