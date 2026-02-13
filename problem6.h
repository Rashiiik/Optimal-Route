#ifndef problem6_H
#define problem6_H

#include "mode.h"

double getWaitingTimeProblem6(int currentTimeMin, Mode mode);

void printProblem6DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon,
                                    int startTimeMin, int deadlineMin);
void runProblem6();

#endif