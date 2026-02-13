#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "csvParse.h"
void printProblem2DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;

    printf("\nProblem No: 2\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    printf("\n");

    // If source is not exactly on a node, show walking segment
    if (fabs(nodes[source].lat - srcLat) > 1e-6 || 
        fabs(nodes[source].lon - srcLon) > 1e-6) {
        double walkDist = haversineDistance(srcLat, srcLon, 
                                           nodes[source].lat, nodes[source].lon);
        printf("Walk from Source (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               srcLon, srcLat, nodes[source].name, nodes[source].lon, nodes[source].lat, walkDist);
        totalDistance += walkDist;
    }

    // Print each segment using the stored edge information
    for (int i = pathLen - 1; i > 0; i--) {
        int from = path[i];
        int to = path[i - 1];
        int edgeIdx = pathEdges[i - 1];  // Edge that leads TO path[i-1]
        
        double distSeg = 0;
        Mode edgeMode = MODE_CAR;
        
        if (edgeIdx >= 0 && edgeIdx < numEdges) {
            distSeg = edges[edgeIdx].distance;
            edgeMode = edges[edgeIdx].mode;
        }

        double rate = (edgeMode == MODE_METRO) ? metroRate : carRate;
        double costSeg = distSeg * rate;
        totalDistance += distSeg;
        totalCost += costSeg;

        printf("Ride %s from %s (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Cost: ৳%.2f\n",
               getModeName(edgeMode),
               nodes[from].name, nodes[from].lon, nodes[from].lat,
               nodes[to].name, nodes[to].lon, nodes[to].lat,
               distSeg, costSeg);
    }

    // If destination is not exactly on a node, show walking segment
    if (fabs(nodes[target].lat - destLat) > 1e-6 || 
        fabs(nodes[target].lon - destLon) > 1e-6) {
        double walkDist = haversineDistance(nodes[target].lat, nodes[target].lon,
                                           destLat, destLon);
        printf("Walk from %s (%.6f, %.6f) to Destination (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               nodes[target].name, nodes[target].lon, nodes[target].lat, destLon, destLat, walkDist);
        totalDistance += walkDist;
    }

    printf("\nTotal Distance: %.3f km\n", totalDistance);
    printf("Total Cost: ৳%.2f\n", totalCost);
}

void runProblem2() {
    double srcLat, srcLon, destLat, destLon;

    printf("Enter source latitude and longitude: ");
    scanf("%lf %lf", &srcLat, &srcLon);

    printf("Enter destination latitude and longitude: ");
    scanf("%lf %lf", &destLat, &destLon);

    int source = findNearestNode(srcLat, srcLon);
    int target = findNearestNode(destLat, destLon);

    if (source == -1 || target == -1) {
        printf("Error: Could not find nodes\n");
        return;
    }

    printf("\nUsing nearest nodes:\n");
    printf("Source Node: %s (%.6f, %.6f)\n", nodes[source].name, nodes[source].lat, nodes[source].lon);
    printf("Target Node: %s (%.6f, %.6f)\n", nodes[target].name, nodes[target].lat, nodes[target].lon);

    // Dijkstra initialization - optimizing for COST
    for (int i = 0; i < numNodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
        prevEdge[i] = -1;  // Track which edge was used
        visited[i] = 0;
    }
    dist[source] = 0;

    double carRate = 20.0;
    double metroRate = 5.0;

    // Dijkstra's algorithm - optimizing for cost
    for (int count = 0; count < numNodes; count++) {
        int u = -1;
        double minCost = INF;

        for (int i = 0; i < numNodes; i++) {
            if (!visited[i] && dist[i] < minCost) {
                minCost = dist[i];
                u = i;
            }
        }

        if (u == -1 || u == target) break;

        visited[u] = 1;

        // Relax edges - considering car and metro
        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u) {
                // Only consider CAR and METRO modes for Problem 2
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_METRO) {
                    continue;
                }
                
                int v = edges[i].to;
                double rate = (edges[i].mode == MODE_METRO) ? metroRate : carRate;
                
                double edgeCost = edges[i].distance * rate;
                double newCost = dist[u] + edgeCost;
                
                if (newCost < dist[v]) {
                    dist[v] = newCost;
                    prev[v] = u;
                    prevEdge[v] = i;  // Remember which edge we used
                }
            }
        }
    }

    // Build path
    int path[MAX_NODES];
    int pathEdges[MAX_NODES];  // Store edge indices
    int pathLen = 0;
    for (int at = target; at != -1; at = prev[at]) {
        path[pathLen] = at;
        pathEdges[pathLen] = prevEdge[at];
        pathLen++;
    }

    if (pathLen == 1 || dist[target] >= INF) {
        printf("No path found between the selected nodes.\n");
        return;
    }

    printf("\nCheapest path found with cost: ৳%.2f\n\n", dist[target]);

    // Print details using the stored edge information
    printProblem2DetailsWithEdges(path, pathEdges, pathLen, source, target, srcLat, srcLon, destLat, destLon);

    exportPathToKML(path, pathLen, "route_problem2.kml");
}