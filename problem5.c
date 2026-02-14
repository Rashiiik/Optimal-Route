#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "timeHandling.h"
#include "csvParse.h"

void printProblem5DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon,
                                    int startTimeMin) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 10.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;
    double totalTravelTime = 0.0;  
    double currentTime = startTimeMin;
    
    char timeBuffer[32];

    printf("\nProblem No: 5\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    formatTime(startTimeMin, timeBuffer, sizeof(timeBuffer));
    printf("Start Time: %s\n", timeBuffer);
    printf("\n");

    if (fabs(nodes[source].lat - srcLat) > 1e-6 || fabs(nodes[source].lon - srcLon) > 1e-6) 
    {
        double walkDist = haversineDistance(srcLat, srcLon, nodes[source].lat, nodes[source].lon);
        double walkTime = (walkDist / WALK_SPEED_KMH) * 60.0;
        
        formatTime((int)currentTime, timeBuffer, sizeof(timeBuffer));
        printf("[%s] Walk from Source (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Time: %.1f min, Cost: ৳0.00\n",
               timeBuffer, srcLon, srcLat, nodes[source].name, nodes[source].lon, nodes[source].lat, 
               walkDist, walkTime);
        totalDistance += walkDist;
        totalTravelTime += walkTime;
        currentTime += walkTime;
    }

    Mode prevMode = MODE_CAR;   // Need to initialize with something
    int isFirstSegment = 1;

    for (int i = pathLen - 1; i > 0; i--) 
    {
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
        if (edgeMode != MODE_CAR && edgeMode != MODE_WALK) 
        {
            if (isFirstSegment || edgeMode != prevMode)         // checking to see if I am boarding a new service
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
        double travelTime = (distSeg / VEHICLE_SPEED_PROBLEM5_KMH) * 60.0;  
        
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
        double walkDist = haversineDistance(nodes[target].lat, nodes[target].lon, destLat, destLon);
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

void runProblem5() {

    double srcLat, srcLon, destLat, destLon;
    char timeInput[32];
    int startTimeMin;

    printf("Enter source latitude and longitude: ");
    scanf("%lf %lf", &srcLat, &srcLon);

    printf("Enter destination latitude and longitude: ");
    scanf("%lf %lf", &destLat, &destLon);
    
    while (getchar() != '\n');
    
    printf("Enter starting time (AM or PM please): ");

    fgets(timeInput, sizeof(timeInput), stdin);
    timeInput[strcspn(timeInput, "\n")] = 0;
    
    startTimeMin = parseTime(timeInput);

    if (startTimeMin < 0) 
    {
        printf("Invalid time format\n");
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

    for (int i = 0; i < numNodes; i++)  // Now we optimize for time
    {
        dist[i] = INF;
        prev[i] = -1;
        prevEdge[i] = -1;
        visited[i] = 0;
        arrivalTime[i] = INF;
    }
    arrivalTime[source] = startTimeMin;

    for (int count = 0; count < numNodes; count++) 
    {
        int u = -1;
        double minTime = INF;

        for (int i = 0; i < numNodes; i++) 
        {
            if (!visited[i] && arrivalTime[i] < minTime)            // Doramumma I have come to bargain
            {
                minTime = arrivalTime[i];
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

        for (int i = 0; i < numEdges; i++) 
        {
            if (edges[i].from == u) 
            {
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
                            continue;  // Service not available
                        }
                    }
                }
                
                double travelTime = (edges[i].distance / VEHICLE_SPEED_PROBLEM5_KMH) * 60.0;
                double newArrivalTime = arrivalTime[u] + waitTime + travelTime;
                
                if (newArrivalTime < arrivalTime[v]) 
                {
                    arrivalTime[v] = newArrivalTime;
                    prev[v] = u;                        // Update if this gives earlier arrival
                    prevEdge[v] = i;
                }
            }
        }
    }

    int path[MAX_NODES];
    int pathEdges[MAX_NODES];
    int pathLen = 0;
    for (int at = target; at != -1; at = prev[at]) {
        path[pathLen] = at;
        pathEdges[pathLen] = prevEdge[at];
        pathLen++;
    }

    if (pathLen == 1 || arrivalTime[target] >= INF) {
        printf("No path found between the selected nodes.\n");
        return;
    }

    double totalTime = arrivalTime[target] - startTimeMin;
    printf("\nFastest path found with travel time: %.1f minutes (%.1f hours)\n\n", totalTime, totalTime / 60.0);

    printProblem5DetailsWithEdges(path, pathEdges, pathLen, source, target, 
                                  srcLat, srcLon, destLat, destLon, startTimeMin);

    exportPathToKML(path, pathLen, "route_problem5.kml");
}