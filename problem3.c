#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "csvParse.h"

int route = 0;

void printProblem3DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 7.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;

    printf("\nProblem No: 3\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    printf("\n");

    if (fabs(nodes[source].lat - srcLat) > 1e-6 || fabs(nodes[source].lon - srcLon) > 1e-6) 
    {
        double walkDist = haversineDistance(srcLat, srcLon, nodes[source].lat, nodes[source].lon);
        printf("Walk from Source (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               srcLon, srcLat, nodes[source].name, nodes[source].lon, nodes[source].lat, walkDist);

        totalDistance += walkDist;
    }

    for (int i = pathLen - 1; i > 0; i--) 
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

        double rate = carRate;

        if (edgeMode == MODE_METRO) rate = metroRate;
        else if (edgeMode == MODE_BIKOLPO) rate = bikolpoRate;
        else if (edgeMode == MODE_UTTARA) rate = uttaraRate;
        
        double costSeg = distSeg * rate;
        totalDistance += distSeg;
        totalCost += costSeg;

        printf("Ride %s from %s (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.3f km, Cost: ৳%.2f\n",
               getModeName(edgeMode),
               nodes[from].name, nodes[from].lon, nodes[from].lat,
               nodes[to].name, nodes[to].lon, nodes[to].lat,
               distSeg, costSeg);
        route++;
    }

    if (fabs(nodes[target].lat - destLat) > 1e-6 || fabs(nodes[target].lon - destLon) > 1e-6) 
    {
        double walkDist = haversineDistance(nodes[target].lat, nodes[target].lon, destLat, destLon);
        printf("Walk from %s (%.6f, %.6f) to Destination (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               nodes[target].name, nodes[target].lon, nodes[target].lat, destLon, destLat, walkDist);

        totalDistance += walkDist;
    }

    printf("\nTotal Distance: %.3f km\n", totalDistance);
    printf("Total Cost: ৳%.2f\n", totalCost);
}

void runProblem3() {

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

    for (int i = 0; i < numNodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
        prevEdge[i] = -1;
        visited[i] = 0;
    }
    dist[source] = 0;

    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 7.0;

    for (int count = 0; count < numNodes; count++) 
    {
        int u = -1;
        double minCost = INF;               // I should switch to cpp

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

        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u) {
                
                if (edges[i].mode != MODE_CAR && edges[i].mode != MODE_METRO && 
                    edges[i].mode != MODE_BIKOLPO && edges[i].mode != MODE_UTTARA) {
                    continue;
                }
                
                int v = edges[i].to;
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

    if (pathLen == 1 || dist[target] >= INF) {
        printf("No path found between the selected nodes.\n");
        return;
    }

    printf("\nCheapest path found with cost: ৳%.2f\n\n", dist[target]);

    printProblem3DetailsWithEdges(path, pathEdges, pathLen, source, target, srcLat, srcLon, destLat, destLon);

    exportPathToKML(path, pathLen, "route_problem3.kml");

    printf("No of routes: %d", route);
}