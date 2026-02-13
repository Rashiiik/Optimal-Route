#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "timeHandling.h"
#include "csvParse.h"

double getWaitingTimeProblem6(int currentTimeMin, Mode mode) {
    // Cars don't need to wait
    if (mode == MODE_CAR || mode == MODE_WALK) {
        return 0.0;
    }
    
    int startTime, endTime, interval;
    
    // Set schedule parameters based on mode
    switch(mode) {
        case MODE_METRO:
            startTime = METRO_START_MIN;
            endTime = METRO_END_MIN;
            interval = METRO_INTERVAL_MIN;
            break;
        case MODE_BIKOLPO:
            startTime = BIKOLPO_START_MIN;
            endTime = BIKOLPO_END_MIN;
            interval = BIKOLPO_INTERVAL_MIN;
            break;
        case MODE_UTTARA:
            startTime = UTTARA_START_MIN;
            endTime = UTTARA_END_MIN;
            interval = UTTARA_INTERVAL_MIN;
            break;
        default:
            return 0.0;
    }
    
    // Check if service is running
    if (currentTimeMin < startTime || currentTimeMin >= endTime) {
        return INF;  // Service not available
    }
    
    // Calculate minutes since service started
    int minutesSinceStart = currentTimeMin - startTime;
    
    // Calculate time to next departure
    int minutesToNext = interval - (minutesSinceStart % interval);
    if (minutesToNext == interval) {
        minutesToNext = 0;  // Service available now
    }
    
    return (double)minutesToNext;
}

void printProblem6DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon,
                                    int startTimeMin, int deadlineMin) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 10.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;
    double totalTravelTime = 0.0;
    double currentTime = startTimeMin;
    
    char timeBuffer[32];
    char deadlineBuffer[32];

    printf("\nProblem No: 6\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    formatTime(startTimeMin, timeBuffer, sizeof(timeBuffer));
    printf("Start Time: %s\n", timeBuffer);
    formatTime(deadlineMin, deadlineBuffer, sizeof(deadlineBuffer));
    printf("Deadline: %s\n", deadlineBuffer);
    printf("\n");

    // If source is not exactly on a node, show walking segment
    if (fabs(nodes[source].lat - srcLat) > 1e-6 || 
        fabs(nodes[source].lon - srcLon) > 1e-6) {
        double walkDist = haversineDistance(srcLat, srcLon, 
                                           nodes[source].lat, nodes[source].lon);
        double walkTime = (walkDist / WALK_SPEED_KMH) * 60.0;
        
        formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
        printf("[%s] Walk from Source (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Time: %.1f min, Cost: ৳0.00\n",
               timeBuffer, srcLon, srcLat, nodes[source].name, nodes[source].lon, nodes[source].lat, 
               walkDist, walkTime);
        totalDistance += walkDist;
        totalTravelTime += walkTime;
        currentTime += walkTime;
    }

    // Track previous mode to detect mode switches
    Mode prevMode = MODE_CAR;
    int isFirstSegment = 1;

    // Print each segment
    for (int i = pathLen - 1; i > 0; i--) {
        int from = path[i];
        int to = path[i - 1];
        int edgeIdx = pathEdges[i - 1];
        
        double distSeg = 0;
        Mode edgeMode = MODE_CAR;
        
        if (edgeIdx >= 0 && edgeIdx < numEdges) {
            distSeg = edges[edgeIdx].distance;
            edgeMode = edges[edgeIdx].mode;
        }

        // Only wait when BOARDING a new scheduled service
        int needsWait = 0;
        if (edgeMode != MODE_CAR && edgeMode != MODE_WALK) {
            if (isFirstSegment || edgeMode != prevMode) {
                needsWait = 1;
            }
        }

        if (needsWait) {
            double waitTime = getWaitingTimeProblem6((int)currentTime, edgeMode);
            if (waitTime > 0 && waitTime < INF) {
                formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
                printf("[%s] Wait for %s: %.0f minutes\n", timeBuffer, getModeName(edgeMode), waitTime);
                currentTime += waitTime;
                totalTravelTime += waitTime;
            }
        }

        double rate = carRate;
        if (edgeMode == MODE_METRO) rate = metroRate;
        else if (edgeMode == MODE_BIKOLPO) rate = bikolpoRate;
        else if (edgeMode == MODE_UTTARA) rate = uttaraRate;
        
        double costSeg = distSeg * rate;
        
        // Calculate travel time based on mode-specific speed
        double speed = CAR_SPEED_PROBLEM6_KMH;
        if (edgeMode == MODE_METRO) speed = METRO_SPEED_PROBLEM6_KMH;
        else if (edgeMode == MODE_BIKOLPO) speed = BIKOLPO_SPEED_PROBLEM6_KMH;
        else if (edgeMode == MODE_UTTARA) speed = UTTARA_SPEED_PROBLEM6_KMH;
        
        double travelTime = (distSeg / speed) * 60.0;
        
        totalDistance += distSeg;
        totalCost += costSeg;
        totalTravelTime += travelTime;

        formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
        printf("[%s] Ride %s from %s (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Time: %.1f min, Cost: ৳%.2f\n",
               timeBuffer, getModeName(edgeMode),
               nodes[from].name, nodes[from].lon, nodes[from].lat,
               nodes[to].name, nodes[to].lon, nodes[to].lat,
               distSeg, travelTime, costSeg);
        
        currentTime += travelTime;
        prevMode = edgeMode;
        isFirstSegment = 0;
    }

    // If destination is not exactly on a node, show walking segment
    if (fabs(nodes[target].lat - destLat) > 1e-6 || 
        fabs(nodes[target].lon - destLon) > 1e-6) {
        double walkDist = haversineDistance(nodes[target].lat, nodes[target].lon,
                                           destLat, destLon);
        double walkTime = (walkDist / WALK_SPEED_KMH) * 60.0;
        
        formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
        printf("[%s] Walk from %s (%.6f, %.6f) to Destination (%.6f, %.6f), Distance: %.3f km, Time: %.1f min, Cost: ৳0.00\n",
               timeBuffer, nodes[target].name, nodes[target].lon, nodes[target].lat, 
               destLon, destLat, walkDist, walkTime);
        totalDistance += walkDist;
        totalTravelTime += walkTime;
        currentTime += walkTime;
    }

    formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
    printf("\nArrival Time: %s\n", timeBuffer);
    
    if (currentTime <= deadlineMin) {
        double slack = deadlineMin - currentTime;
        printf("Status: ON TIME (%.0f minutes early)\n", slack);
    } else {
        double late = currentTime - deadlineMin;
        printf("Status: LATE (%.0f minutes late)\n", late);
    }
    
    printf("Total Distance: %.3f km\n", totalDistance);
    printf("Total Travel Time: %.1f minutes (%.1f hours)\n", totalTravelTime, totalTravelTime / 60.0);
    printf("Total Cost: ৳%.2f\n", totalCost);
}

void runProblem6() {
    double srcLat, srcLon, destLat, destLon;
    char timeInput[32];
    char deadlineInput[32];
    int startTimeMin, deadlineMin;

    printf("Enter source latitude and longitude: ");
    scanf("%lf %lf", &srcLat, &srcLon);

    printf("Enter destination latitude and longitude: ");
    scanf("%lf %lf", &destLat, &destLon);
    
    // Clear input buffer
    while (getchar() != '\n');
    
    printf("Enter starting time (e.g., '5:43 PM' or '9:30 AM'): ");
    fgets(timeInput, sizeof(timeInput), stdin);
    timeInput[strcspn(timeInput, "\n")] = 0;
    
    startTimeMin = parseTime(timeInput);
    if (startTimeMin < 0) {
        printf("Invalid time format\n");
        return;
    }
    
    printf("Enter deadline time (e.g., '8:12 PM' or '11:00 AM'): ");
    fgets(deadlineInput, sizeof(deadlineInput), stdin);
    deadlineInput[strcspn(deadlineInput, "\n")] = 0;
    
    deadlineMin = parseTime(deadlineInput);
    if (deadlineMin < 0) {
        printf("Invalid deadline format\n");
        return;
    }
    
    // Check if deadline is after start time
    if (deadlineMin <= startTimeMin) {
        printf("Error: Deadline must be after start time\n");
        return;
    }

    int source = findNearestNode(srcLat, srcLon);
    int target = findNearestNode(destLat, destLon);

    if (source == -1 || target == -1) {
        printf("Error: Could not find nodes\n");
        return;
    }

    printf("\nUsing nearest nodes:\n");
    printf("Source Node: %s (%.6f, %.6f)\n", nodes[source].name, nodes[source].lat, nodes[source].lon);
    printf("Target Node: %s (%.6f, %.6f)\n", nodes[target].name, nodes[target].lat, nodes[target].lon);

    // Dijkstra initialization - optimizing for COST with deadline constraint
    for (int i = 0; i < numNodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
        prevEdge[i] = -1;
        visited[i] = 0;
        arrivalTime[i] = INF;
    }
    dist[source] = 0;
    arrivalTime[source] = startTimeMin;

    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 10.0;

    // Dijkstra's algorithm - optimizing for cost with deadline constraint
    for (int count = 0; count < numNodes; count++) {
        int u = -1;
        double minCost = INF;

        // Find unvisited node with minimum cost that can still meet deadline
        for (int i = 0; i < numNodes; i++) {
            if (!visited[i] && dist[i] < minCost) {
                minCost = dist[i];
                u = i;
            }
        }

        if (u == -1 || u == target) break;

        visited[u] = 1;

        // Determine the mode used to ARRIVE at node u
        Mode arrivalMode = MODE_CAR;
        if (prevEdge[u] >= 0 && prevEdge[u] < numEdges) {
            arrivalMode = edges[prevEdge[u]].mode;
        }

        // Relax edges
        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u) {
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_METRO && 
                    edges[i].mode != MODE_BIKOLPO && edges[i].mode != MODE_UTTARA) {
                    continue;
                }
                
                int v = edges[i].to;
                
                // Calculate waiting time when boarding (mode switch)
                double waitTime = 0.0;
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_WALK) {
                    if (edges[i].mode != arrivalMode || u == source) {
                        waitTime = getWaitingTimeProblem6((int)arrivalTime[u], edges[i].mode);
                        if (waitTime >= INF) {
                            continue;  // Service not available
                        }
                    }
                }
                
                // Calculate travel time with mode-specific speeds
                double speed = CAR_SPEED_PROBLEM6_KMH;
                if (edges[i].mode == MODE_METRO) speed = METRO_SPEED_PROBLEM6_KMH;
                else if (edges[i].mode == MODE_BIKOLPO) speed = BIKOLPO_SPEED_PROBLEM6_KMH;
                else if (edges[i].mode == MODE_UTTARA) speed = UTTARA_SPEED_PROBLEM6_KMH;
                
                double travelTime = (edges[i].distance / speed) * 60.0;
                double newArrivalTime = arrivalTime[u] + waitTime + travelTime;
                
                // DEADLINE CONSTRAINT: Skip this edge if it would make us late
                if (newArrivalTime > deadlineMin) {
                    continue;  // Would miss deadline
                }
                
                // Get cost rate
                double rate = carRate;
                if (edges[i].mode == MODE_METRO) rate = metroRate;
                else if (edges[i].mode == MODE_BIKOLPO) rate = bikolpoRate;
                else if (edges[i].mode == MODE_UTTARA) rate = uttaraRate;
                
                double edgeCost = edges[i].distance * rate;
                double newCost = dist[u] + edgeCost;
                
                // Update if cheaper and meets deadline
                if (newCost < dist[v]) {
                    dist[v] = newCost;
                    prev[v] = u;
                    prevEdge[v] = i;
                    arrivalTime[v] = newArrivalTime;
                }
            }
        }
    }

    // Build path
    int path[MAX_NODES];
    int pathEdges[MAX_NODES];
    int pathLen = 0;
    for (int at = target; at != -1; at = prev[at]) {
        path[pathLen] = at;
        pathEdges[pathLen] = prevEdge[at];
        pathLen++;
    }

    if (pathLen == 1 || dist[target] >= INF) {
        printf("No path found that meets the deadline constraint.\n");
        return;
    }

    printf("\nCheapest deadline-constrained path found with cost: ৳%.2f\n\n", dist[target]);

    printProblem6DetailsWithEdges(path, pathEdges, pathLen, source, target, 
                                  srcLat, srcLon, destLat, destLon, startTimeMin, deadlineMin);

    exportPathToKML(path, pathLen, "route_problem6.kml");
}