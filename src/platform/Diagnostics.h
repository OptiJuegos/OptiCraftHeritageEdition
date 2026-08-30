#pragma once

const char* platformOomDiagnosticLine(int index);
void platformMemoryCheckpoint(const char* tag);
void platformHardwareCheckpoint(const char* tag);
void platformCaptureBadAlloc();

// Free heap KB, or -1 where the platform exposes no cheap answer. Callers that
// only want a log line should use platformMemoryCheckpoint(); this exists for
// the ones that need the number itself, such as the per-stage deltas in
// platform/WorldLoadTrace.cpp.
long platformHeapFreeKb();
