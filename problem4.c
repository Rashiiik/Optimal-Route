#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "timeHandling.h"
#include "csvParse.h"

void printProblem4DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon,
                                    int startTimeMin) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 10.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;
    double totalTravelTime = 0.0;  // in minutes
    double currentTime = startTimeMin;
    
    char timeBuffer[32];                // what is this for you may ask. I do not know

    printf("\nProblem No: 4\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    formatTime(startTimeMin, timeBuffer, sizeof(timeBuffer));
    printf("Start Time: %s\n", timeBuffer);
    printf("\n");

    if (fabs(nodes[source].lat - srcLat) > 1e-6 || fabs(nodes[source].lon - srcLon) > 1e-6) {
        double walkDist = haversineDistance(srcLat, srcLon, 
                                           nodes[source].lat, nodes[source].lon);
        double walkTime = (walkDist / WALK_SPEED_KMH) * 60.0;
        
        formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));           // Ahh its for time format
        printf("[%s] Walk from Source (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Time: %.1f min, Cost: ৳0.00\n",
               timeBuffer, srcLon, srcLat, nodes[source].name, nodes[source].lon, nodes[source].lat, 
               walkDist, walkTime);
        totalDistance += walkDist;
        totalTravelTime += walkTime;
        currentTime += walkTime;
    }

    Mode prevMode = MODE_CAR; 
    int isFirstSegment = 1;

    for (int i = pathLen - 1; i > 0; i--)       // We print the segments using the stored edge info
    {
        int from = path[i];
        int to = path[i - 1];
        int edgeIdx = pathEdges[i - 1];
        
        double distSeg = 0;
        Mode edgeMode = MODE_CAR;
        
        if (edgeIdx >= 0 && edgeIdx < numEdges) 
        {
            distSeg = edges[edgeIdx].distance;
            edgeMode = edges[edgeIdx].mode;
        }

        int needsWait = 0;
        if (edgeMode != MODE_CAR && edgeMode != MODE_WALK) 
        {
            if (isFirstSegment || edgeMode != prevMode)         // Need to wait if: first segment on this mode, or switching from a different mode
            {
                needsWait = 1;
            }
        }

        if (needsWait) 
        {
            double waitTime = getWaitingTime((int)currentTime, edgeMode);

            if (waitTime > 0 && waitTime < INF) 
            {
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
        double travelTime = (distSeg / VEHICLE_SPEED_KMH) * 60.0;
        
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

    if (fabs(nodes[target].lat - destLat) > 1e-6 || fabs(nodes[target].lon - destLon) > 1e-6) 
    {
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
    printf("Total Distance: %.3f km\n", totalDistance);
    printf("Total Travel Time: %.1f minutes (%.1f hours)\n", totalTravelTime, totalTravelTime / 60.0);
    printf("Total Cost: ৳%.2f\n", totalCost);
}

void runProblem4() {
    double srcLat, srcLon, destLat, destLon;
    char timeInput[32];
    int startTimeMin;

    printf("Enter source latitude and longitude: ");
    scanf("%lf %lf", &srcLat, &srcLon);

    printf("Enter destination latitude and longitude: ");
    scanf("%lf %lf", &destLat, &destLon);
    
    while (getchar() != '\n');          // Cleaning the input buffer else it produces garbage
    
    printf("Enter starting time (AM or PM please): ");
    fgets(timeInput, sizeof(timeInput), stdin);
    timeInput[strcspn(timeInput, "\n")] = 0;
    
    startTimeMin = parseTime(timeInput);
    if (startTimeMin < 0) 
    {
        printf("e.g 9:30 AM\n");
        return;
    }

    int source = findNearestNode(srcLat, srcLon);
    int target = findNearestNode(destLat, destLon);

    if (source == -1 || target == -1) 
    {
        printf("Error: Could not find nodes\n");
        return;
    }

    printf("\nUsing nearest nodes:\n");
    printf("Source Node: %s (%.6f, %.6f)\n", nodes[source].name, nodes[source].lat, nodes[source].lon);
    printf("Target Node: %s (%.6f, %.6f)\n", nodes[target].name, nodes[target].lat, nodes[target].lon);

    for (int i = 0; i < numNodes; i++) 
    {
        dist[i] = INF;
        prev[i] = -1;                   // I am just copy pasting the same code
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

    for (int count = 0; count < numNodes; count++) 
    {
        int u = -1;
        double minCost = INF;               // Cpp is good but C is life

        for (int i = 0; i < numNodes; i++) 
        {
            if (!visited[i] && dist[i] < minCost) 
            {
                minCost = dist[i];
                u = i;
            }
        }

        if (u == -1 || u == target) break;

        visited[u] = 1;

        Mode arrivalMode = MODE_CAR;  

        if (prevEdge[u] >= 0 && prevEdge[u] < numEdges) 
        {
            arrivalMode = edges[prevEdge[u]].mode;
        }

        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u) {
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_METRO && 
                    edges[i].mode != MODE_BIKOLPO && edges[i].mode != MODE_UTTARA) {
                    continue;
                }
                
                int v = edges[i].to;
                
                double waitTime = 0.0;
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_WALK) 
                {
                    if (edges[i].mode != arrivalMode || u == source) 
                    {
                        waitTime = getWaitingTime((int)arrivalTime[u], edges[i].mode);
                        if (waitTime >= INF) 
                        {
                            continue;  
                        }
                    }
                    // else continuing on same vehicle, no wait
                }
                
                double travelTime = (edges[i].distance / VEHICLE_SPEED_KMH) * 60.0;
                double newArrivalTime = arrivalTime[u] + waitTime + travelTime;
                
                double rate = carRate;
                if (edges[i].mode == MODE_METRO) rate = metroRate;
                else if (edges[i].mode == MODE_BIKOLPO) rate = bikolpoRate;
                else if (edges[i].mode == MODE_UTTARA) rate = uttaraRate;
                
                double edgeCost = edges[i].distance * rate;
                double newCost = dist[u] + edgeCost;
                
                if (newCost < dist[v]) 
                {
                    dist[v] = newCost;
                    prev[v] = u;
                    prevEdge[v] = i;
                    arrivalTime[v] = newArrivalTime;
                }
            }
        }
    }

    int path[MAX_NODES];
    int pathEdges[MAX_NODES];
    int pathLen = 0;
    for (int at = target; at != -1; at = prev[at]) 
    {
        path[pathLen] = at;
        pathEdges[pathLen] = prevEdge[at];
        pathLen++;
    }

    if (pathLen == 1 || dist[target] >= INF) 
    {
        printf("No path found between the selected nodes.\n");
        return;
    }

    printf("\nCheapest time-constrained path found with cost: ৳%.2f\n\n", dist[target]);

    printProblem4DetailsWithEdges(path, pathEdges, pathLen, source, target, 
                                  srcLat, srcLon, destLat, destLon, startTimeMin);

    exportPathToKML(path, pathLen, "route_problem4.kml");
}