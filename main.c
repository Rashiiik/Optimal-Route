#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define MAX_NODES 100000
#define INF 9999999999.0
#define MAX_LINE 200000
#define MAX_TOKENS 5000
#define EARTH_RADIUS_KM 6371.0
#define PI 3.14159265358979323846

double dist[MAX_NODES];
int prev[MAX_NODES];
int visited[MAX_NODES];

typedef enum
{
    MODE_WALK,
    MODE_METRO,
    MODE_CAR,
    MODE_BIKOLPO,
    MODE_UTTARA
} Mode;

typedef struct 
{
    int from;
    int to;
    Mode mode;
    double distance;  
    double cost;      
    double speed;     
} Edge;

typedef struct 
{
    int id;
    char name[32];
    double lat;
    double lon;
} Node;

Node nodes[MAX_NODES];
Edge edges[MAX_NODES*10];

int numNodes = 0;
int numEdges = 0;

// Haversine formula to calculate distance between two lat-lon points
double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
    
    lat1 = lat1 * PI / 180.0;
    lat2 = lat2 * PI / 180.0;
    
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1) * cos(lat2) *
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return EARTH_RADIUS_KM * c;
}

static void trim_in_place(char *s) {
    if (!s) return;
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

static int split_csv(char *line, char **tokens, int maxTokens) {
    int count = 0;
    char *save = NULL;
    for (char *tok = strtok_r(line, ",", &save);
         tok && count < maxTokens;
         tok = strtok_r(NULL, ",", &save)) {
        trim_in_place(tok);
        tokens[count++] = tok;
    }
    return count;
}

static int is_number_token(const char *s) {
    if (!s) return 0;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;
    char *end = NULL;
    (void)strtod(s, &end);
    while (end && *end && isspace((unsigned char)*end)) end++;
    return end && *end == '\0';
}

int findOrAddNode(double lat, double lon) {
    for (int i = 0; i < numNodes; i++) {
        if (fabs(nodes[i].lat - lat) < 1e-6 &&
            fabs(nodes[i].lon - lon) < 1e-6)
            return i;
    }
    // Add new node
    nodes[numNodes].id = numNodes;
    nodes[numNodes].lat = lat;
    nodes[numNodes].lon = lon;
    sprintf(nodes[numNodes].name, "Node%d", numNodes);
    return numNodes++;
}

int findNearestNode(double lat, double lon) {
    int best = -1;
    double bestDist = 1e12;

    for (int i = 0; i < numNodes; i++) {
        double d = haversineDistance(lat, lon, nodes[i].lat, nodes[i].lon);
        if (d < bestDist) {
            bestDist = d;
            best = i;
        }
    }

    return best;
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

void parseRoadmapCSV(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { 
        printf("Error opening %s\n", filename); 
        return; 
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];
    int roadCount = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;
        
        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 6) continue; // type + at least 1 pair + altitude + length

        const char *altTok = tokens[count - 2];
        const char *lenTok = tokens[count - 1];
        if (!is_number_token(altTok) || !is_number_token(lenTok)) continue;

        // Coordinates are from tokens[1] to tokens[count-3]
        int coordCount = count - 3;
        if (coordCount < 4 || coordCount % 2 != 0) continue;

        roadCount++;
        
        // Process consecutive lon-lat pairs
        // Format: DhakaStreet, lon1, lat1, lon2, lat2, ..., altitude, length
        for (int i = 1; i + 3 <= count - 2; i += 2) {
            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i+1]);
            double lon2 = atof(tokens[i+2]);
            double lat2 = atof(tokens[i+3]);

            int from = findOrAddNode(lat1, lon1);
            int to = findOrAddNode(lat2, lon2);

            // Calculate actual segment distance using Haversine
            double segmentDist = haversineDistance(lat1, lon1, lat2, lon2);

            // Add bidirectional edges (roads go both ways)
            addEdge(from, to, MODE_CAR, segmentDist);
            addEdge(to, from, MODE_CAR, segmentDist);
        }
    }

    fclose(f);
    printf("Parsed %d roads, created %d nodes and %d car edges from %s\n", 
           roadCount, numNodes, numEdges, filename);
}

void exportPathToKML(int path[], int pathLen, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { 
        printf("Failed to open %s\n", filename); 
        return; 
    }

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<kml xmlns=\"http://earth.google.com/kml/2.1\">\n");
    fprintf(f, "<Document>\n");
    fprintf(f, "<Placemark>\n");
    fprintf(f, "<name>Route</name>\n");
    fprintf(f, "<LineString>\n");
    fprintf(f, "<tessellate>1</tessellate>\n");
    fprintf(f, "<coordinates>\n");

    // Write path from start to end
    for (int i = pathLen - 1; i >= 0; i--) {
        int nodeId = path[i];
        fprintf(f, "%.6f,%.6f,0\n", nodes[nodeId].lon, nodes[nodeId].lat);
    }

    fprintf(f, "</coordinates>\n");
    fprintf(f, "</LineString>\n");
    fprintf(f, "</Placemark>\n");
    fprintf(f, "</Document>\n");
    fprintf(f, "</kml>\n");
    fclose(f);

    printf("Exported path to %s\n", filename);
}

void printProblem1Details(int path[], int pathLen, int source, int target, 
                          double srcLat, double srcLon, double destLat, double destLon) {
    double carRate = 20.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;

    printf("\nProblem No: 1\n");
    printf("Source: (%.6f, %.6f)\n", srcLon, srcLat);
    printf("Destination: (%.6f, %.6f)\n", destLon, destLat);
    printf("\n");

    // If source is not exactly on a node, show walking segment
    if (fabs(nodes[source].lat - srcLat) > 1e-6 || 
        fabs(nodes[source].lon - srcLon) > 1e-6) {
        double walkDist = haversineDistance(srcLat, srcLon, 
                                           nodes[source].lat, nodes[source].lon);
        printf("Walk from Source (%.6f, %.6f) to (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               srcLon, srcLat, nodes[source].lon, nodes[source].lat, walkDist);
        totalDistance += walkDist;
    }

    // Print each car segment
    for (int i = pathLen - 1; i > 0; i--) {
        int from = path[i];
        int to = path[i - 1];

        // Find the edge distance
        double distSeg = 0;
        for (int j = 0; j < numEdges; j++) {
            if (edges[j].from == from && edges[j].to == to && edges[j].mode == MODE_CAR) {
                distSeg = edges[j].distance;
                break;
            }
        }

        double costSeg = distSeg * carRate;
        totalDistance += distSeg;
        totalCost += costSeg;

        printf("Ride Car from (%.6f, %.6f) to (%.6f, %.6f), Distance: %.3f km, Cost: ৳%.2f\n",
               nodes[from].lon, nodes[from].lat,
               nodes[to].lon, nodes[to].lat,
               distSeg, costSeg);
    }

    // If destination is not exactly on a node, show walking segment
    if (fabs(nodes[target].lat - destLat) > 1e-6 || 
        fabs(nodes[target].lon - destLon) > 1e-6) {
        double walkDist = haversineDistance(nodes[target].lat, nodes[target].lon,
                                           destLat, destLon);
        printf("Walk from (%.6f, %.6f) to Destination (%.6f, %.6f), Distance: %.3f km, Cost: ৳0.00\n",
               nodes[target].lon, nodes[target].lat, destLon, destLat, walkDist);
        totalDistance += walkDist;
    }

    printf("\nTotal Distance: %.3f km\n", totalDistance);
    printf("Total Cost: ৳%.2f\n", totalCost);
}

void runProblem1() {
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

    printf("\nUsing nearest roadmap nodes:\n");
    printf("Source Node: %s (%.6f, %.6f)\n", nodes[source].name, nodes[source].lat, nodes[source].lon);
    printf("Target Node: %s (%.6f, %.6f)\n", nodes[target].name, nodes[target].lat, nodes[target].lon);

    // Dijkstra initialization
    for (int i = 0; i < numNodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[source] = 0;

    // Dijkstra's algorithm
    for (int count = 0; count < numNodes; count++) {
        int u = -1;
        double minDist = INF;

        for (int i = 0; i < numNodes; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1 || u == target) break;

        visited[u] = 1;

        // Relax edges
        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u && edges[i].mode == MODE_CAR) {
                int v = edges[i].to;
                double newDist = dist[u] + edges[i].distance;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                }
            }
        }
    }

    // Build path
    int path[MAX_NODES];
    int pathLen = 0;
    for (int at = target; at != -1; at = prev[at]) {
        path[pathLen++] = at;
    }

    if (pathLen == 1 || dist[target] >= INF) {
        printf("No path found between the selected nodes.\n");
        return;
    }

    printf("\nShortest path found with distance: %.3f km\n\n", dist[target]);

    printProblem1Details(path, pathLen, source, target, srcLat, srcLon, destLat, destLon);

    exportPathToKML(path, pathLen, "route.kml");
}

int main() {
    parseRoadmapCSV("Roadmap-Dhaka.csv");

    if (numNodes == 0) {
        printf("No nodes loaded. Please check your CSV file.\n");
        return 1;
    }

    while (1) {
        printf("\n-------Mr Efficient--------\n");
        printf("[1] Shortest Car Route\n");
        printf("[7] Quit\n");
        printf("-----------------------------\n");

        int choice;
        printf("Enter Choice: ");
        scanf("%d", &choice);
        printf("-----------------------------\n");

        if (choice == 7) {
            break;
        }

        switch (choice) {
        case 1:
            runProblem1();
            break;
        default:
            printf("Invalid choice\n");
            break;
        }
    }
    
    return 0;
}