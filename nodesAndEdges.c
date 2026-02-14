#include <math.h>
#include <stdio.h>
#include "nodesAndEdges.h"
#include "mode.h"

double dist[MAX_NODES];
int prev[MAX_NODES];
int visited[MAX_NODES];
int prevEdge[MAX_NODES];

Node nodes[MAX_NODES];
Edge edges[MAX_NODES*10];

int numNodes = 0;
int numEdges = 0;

int findOrAddNode(double lat, double lon) {

    double tolerance = 0.0001;  // 1e^-6 that is if the node is within that distance we join them
    
    for (int i = 0; i < numNodes; i++) 
    {
        if (fabs(nodes[i].lat - lat) < tolerance &&
            fabs(nodes[i].lon - lon) < tolerance)
            return i;
    }
    
    nodes[numNodes].id = numNodes;
    nodes[numNodes].lat = lat;
    nodes[numNodes].lon = lon;
    sprintf(nodes[numNodes].name, "Node%d", numNodes);

    return numNodes++;
}

int findNearestNode(double lat, double lon) {               // self explanatory

    int best = -1;
    double bestDist = 1e12;

    for (int i = 0; i < numNodes; i++) 
    {
        double d = haversineDistance(lat, lon, nodes[i].lat, nodes[i].lon);

        if (d < bestDist) 
        {
            bestDist = d;
            best = i;
        }
    }

    return best;
}

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {

    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
    
    lat1 = lat1 * PI / 180.0;
    lat2 = lat2 * PI / 180.0;
    
    double a = sin(dLat/2) * sin(dLat/2) + cos(lat1) * cos(lat2) * sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return EARTH_RADIUS_KM * c;  // Oval
}

void addEdge(int from, int to, Mode mode, double distance) {
    
    edges[numEdges].from = from;
    edges[numEdges].to = to;
    edges[numEdges].mode = mode;
    edges[numEdges].distance = distance;
    edges[numEdges].cost = 0;
    edges[numEdges].speed = 30;
    numEdges++;
}