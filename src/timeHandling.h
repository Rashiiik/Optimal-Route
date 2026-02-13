#ifndef timeHandling_H
#define timeHandling_H

#include "mode.h"
#include "nodesAndEdges.h"

extern double arrivalTime[MAX_NODES];

int parseTime(const char* timeStr);
void formatTime(int minutes, char* buffer, int bufferSize);
double getWaitingTime(int currentTimeMin, Mode mode);

#endif
