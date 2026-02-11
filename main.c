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
#define WALK_SPEED_KMH 2.0
#define MAX_WALK_DISTANCE_KM 0.1  // Maximum 500 meters walking distance

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
    // Use a larger tolerance to merge nearby nodes (about 10 meters)
    double tolerance = 0.0001;  // ~10 meters instead of 1e-6 (~0.1 meters)
    
    for (int i = 0; i < numNodes; i++) {
        if (fabs(nodes[i].lat - lat) < tolerance &&
            fabs(nodes[i].lon - lon) < tolerance)
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


// Add this function to get mode name as string
const char* getModeName(Mode mode) {
    switch(mode) {
        case MODE_WALK: return "Walk";
        case MODE_METRO: return "Metro";
        case MODE_CAR: return "Car";
        case MODE_BIKOLPO: return "Bikolpo Bus";
        case MODE_UTTARA: return "Uttara Bus";
        default: return "Unknown";
    }
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

void addWalkingEdges() {
    int walkEdgeCount = 0;
    
    printf("Adding walking edges to connect different transport modes...\n");
    
    // Identify which nodes are metro stations
    int *isMetroStation = (int*)calloc(numNodes, sizeof(int));
    int metroStationCount = 0;
    
    for (int i = 0; i < numEdges; i++) {
        if (edges[i].mode == MODE_METRO) {
            if (!isMetroStation[edges[i].from]) {
                isMetroStation[edges[i].from] = 1;
                metroStationCount++;
            }
            if (!isMetroStation[edges[i].to]) {
                isMetroStation[edges[i].to] = 1;
                metroStationCount++;
            }
        }
    }
    
    printf("Found %d metro stations\n", metroStationCount);
    
    // For each metro station, find the closest 3 road nodes within 200m
    #define MAX_CONNECTIONS_PER_STATION 3
    #define MAX_WALK_DIST 0.2  // 200 meters
    
    for (int i = 0; i < numNodes; i++) {
        if (!isMetroStation[i]) continue;
        
        // Store distances to nearby road nodes
        typedef struct {
            int nodeId;
            double distance;
        } NearbyNode;
        
        NearbyNode nearby[100];
        int nearbyCount = 0;
        
        // Find all road nodes within walking distance
        for (int j = 0; j < numNodes; j++) {
            if (isMetroStation[j]) continue; // Skip other metro stations
            
            double walkDist = haversineDistance(
                nodes[i].lat, nodes[i].lon,
                nodes[j].lat, nodes[j].lon
            );
            
            if (walkDist <= MAX_WALK_DIST && walkDist > 0.0001) {
                if (nearbyCount < 100) {
                    nearby[nearbyCount].nodeId = j;
                    nearby[nearbyCount].distance = walkDist;
                    nearbyCount++;
                }
            }
        }
        
        // Sort by distance and take only the closest MAX_CONNECTIONS_PER_STATION
        for (int a = 0; a < nearbyCount - 1; a++) {
            for (int b = a + 1; b < nearbyCount; b++) {
                if (nearby[b].distance < nearby[a].distance) {
                    NearbyNode temp = nearby[a];
                    nearby[a] = nearby[b];
                    nearby[b] = temp;
                }
            }
        }
        
        // Add walking edges to the closest nodes only
        int maxConnections = (nearbyCount < MAX_CONNECTIONS_PER_STATION) ? nearbyCount : MAX_CONNECTIONS_PER_STATION;
        for (int k = 0; k < maxConnections; k++) {
            int j = nearby[k].nodeId;
            double walkDist = nearby[k].distance;
            
            addEdge(i, j, MODE_WALK, walkDist);
            addEdge(j, i, MODE_WALK, walkDist);
            walkEdgeCount += 2;
        }
    }
    
    free(isMetroStation);
    printf("Added %d walking edges (max %d connections per station, max %.1f km walk)\n", 
           walkEdgeCount, MAX_CONNECTIONS_PER_STATION, MAX_WALK_DIST);
}

int prevEdge[MAX_NODES]; 

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

void parseMetroCSV(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { 
        printf("Error opening %s\n", filename); 
        return; 
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];
    int routeCount = 0;
    int metroEdgesBefore = numEdges;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;
        
        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 5) continue;

        // Last 2 tokens should be station names (non-numeric)
        const char *startStation = tokens[count - 2];
        const char *endStation = tokens[count - 1];

        // Verify they are not numbers
        if (is_number_token(startStation) || is_number_token(endStation)) {
            continue;
        }

        // Coordinates are from tokens[1] to tokens[count-3]
        int coordCount = count - 3; // Exclude type name and 2 station names
        if (coordCount < 4 || coordCount % 2 != 0) {
            continue;
        }

        routeCount++;
        int stationsOnThisRoute = 0;
        
        // Process consecutive lon-lat pairs for metro stations
        // Format: DhakaMetroRail, lon1, lat1, lon2, lat2, ..., StartStation, EndStation
        for (int i = 1; i + 3 <= count - 2; i += 2) {
            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i+1]);
            double lon2 = atof(tokens[i+2]);
            double lat2 = atof(tokens[i+3]);

            int from = findOrAddNode(lat1, lon1);
            int to = findOrAddNode(lat2, lon2);

            // Calculate actual segment distance
            double segmentDist = haversineDistance(lat1, lon1, lat2, lon2);

            // Add bidirectional metro edges
            addEdge(from, to, MODE_METRO, segmentDist);
            addEdge(to, from, MODE_METRO, segmentDist);
            stationsOnThisRoute++;
            
            // Set station names for first and last stops
            if (i == 1) {
                strncpy(nodes[from].name, startStation, sizeof(nodes[from].name) - 1);
                nodes[from].name[sizeof(nodes[from].name) - 1] = '\0';
            }
            if (i + 4 > count - 2) { // Last pair
                strncpy(nodes[to].name, endStation, sizeof(nodes[to].name) - 1);
                nodes[to].name[sizeof(nodes[to].name) - 1] = '\0';
            }
        }
        
        printf("  Route from %s to %s: %d segments\n", startStation, endStation, stationsOnThisRoute);
    }

    fclose(f);
    int metroEdgesAdded = numEdges - metroEdgesBefore;
    printf("Parsed %d metro routes, added %d metro edges from %s\n", 
           routeCount, metroEdgesAdded, filename);
}

void parseBusCSV(const char *filename, Mode busMode) {
    FILE *f = fopen(filename, "r");
    if (!f) { 
        printf("Error opening %s\n", filename); 
        return; 
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];
    int routeCount = 0;
    int busEdgesBefore = numEdges;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;
        
        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 5) continue;

        // Last 2 tokens should be station names (non-numeric)
        const char *startStation = tokens[count - 2];
        const char *endStation = tokens[count - 1];

        // Verify they are not numbers
        if (is_number_token(startStation) || is_number_token(endStation)) {
            continue;
        }

        // Coordinates are from tokens[1] to tokens[count-3]
        int coordCount = count - 3;
        if (coordCount < 4 || coordCount % 2 != 0) {
            continue;
        }

        routeCount++;
        int stopsOnThisRoute = 0;
        
        // Process consecutive lon-lat pairs for bus stops
        for (int i = 1; i + 3 <= count - 2; i += 2) {
            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i+1]);
            double lon2 = atof(tokens[i+2]);
            double lat2 = atof(tokens[i+3]);

            int from = findOrAddNode(lat1, lon1);
            int to = findOrAddNode(lat2, lon2);

            // Calculate actual segment distance
            double segmentDist = haversineDistance(lat1, lon1, lat2, lon2);

            // Add bidirectional bus edges
            addEdge(from, to, busMode, segmentDist);
            addEdge(to, from, busMode, segmentDist);
            stopsOnThisRoute++;
            
            // Set station names for first and last stops
            if (i == 1) {
                strncpy(nodes[from].name, startStation, sizeof(nodes[from].name) - 1);
                nodes[from].name[sizeof(nodes[from].name) - 1] = '\0';
            }
            if (i + 4 > count - 2) { // Last pair
                strncpy(nodes[to].name, endStation, sizeof(nodes[to].name) - 1);
                nodes[to].name[sizeof(nodes[to].name) - 1] = '\0';
            }
        }
        
        printf("  Route from %s to %s: %d segments\n", startStation, endStation, stopsOnThisRoute);
    }

    fclose(f);
    int busEdgesAdded = numEdges - busEdgesBefore;
    const char* busName = (busMode == MODE_BIKOLPO) ? "Bikolpo" : "Uttara";
    printf("Parsed %d %s bus routes, added %d bus edges from %s\n", 
           routeCount, busName, busEdgesAdded, filename);
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

void printProblem3DetailsWithEdges(int path[], int pathEdges[], int pathLen, int source, int target, 
                                    double srcLat, double srcLon, double destLat, double destLon) {
    double carRate = 20.0;
    double metroRate = 5.0;
    double bikolpoRate = 7.0;
    double uttaraRate = 10.0;
    double totalDistance = 0.0;
    double totalCost = 0.0;

    printf("\nProblem No: 3\n");
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
        int edgeIdx = pathEdges[i - 1];
        
        double distSeg = 0;
        Mode edgeMode = MODE_CAR;
        
        if (edgeIdx >= 0 && edgeIdx < numEdges) {
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

    // Dijkstra initialization - optimizing for COST
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
    double uttaraRate = 10.0;

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

        // Relax edges - considering car, metro, and buses
        for (int i = 0; i < numEdges; i++) {
            if (edges[i].from == u) {
                // Only consider CAR, METRO, BIKOLPO, and UTTARA modes for Problem 3
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
                
                if (newCost < dist[v]) {
                    dist[v] = newCost;
                    prev[v] = u;
                    prevEdge[v] = i;
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
        printf("No path found between the selected nodes.\n");
        return;
    }

    printf("\nCheapest path found with cost: ৳%.2f\n\n", dist[target]);

    printProblem3DetailsWithEdges(path, pathEdges, pathLen, source, target, srcLat, srcLon, destLat, destLon);

    exportPathToKML(path, pathLen, "route_problem3.kml");
}

int main() {
    parseRoadmapCSV("Roadmap-Dhaka.csv");
    parseMetroCSV("Routemap-DhakaMetroRail.csv");
    parseBusCSV("Routemap-BikolpoBus.csv", MODE_BIKOLPO);
    parseBusCSV("Routemap-UttaraBus.csv", MODE_UTTARA);

    //WalkingEdges();

    if (numNodes == 0) {
        printf("No nodes loaded. Please check your CSV file.\n");
        return 1;
    }

    while (1) {
        printf("\n-------Mr Efficient--------\n");
        printf("[1] Shortest Car Route [Problem 1]\n");
        printf("[2] Cheapest Route(Car and Metro) [Problem 2]\n");
        printf("[3] Cheapest Route(Car, Metro and Bus) [Problem 3]\n");
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
        case 2:
            runProblem2();
            break;
        case 3:
            runProblem3();
            break;
        default:
            printf("Invalid choice\n");
            break;
        }
    }
    
    return 0;
}