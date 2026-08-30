#pragma once

void wiiWorldRendererBuildStats(unsigned int *rawPeakBytes,
                                unsigned int *poolBytes,
                                unsigned int *poolHits,
                                unsigned int *poolMisses,
                                unsigned int *poolOverflows);
void wiiWorldRendererBuildPoolTrim();
