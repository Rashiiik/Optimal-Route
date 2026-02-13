#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "csvParse.h"
#include "mode.h"
#include "nodesAndEdges.h"

void trim_in_place(char *s) {
    if (!s) return;
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

int split_csv(char *line, char **tokens, int maxTokens) {
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

int is_number_token(const char *s) {
    if (!s) return 0;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;
    char *end = NULL;
    (void)strtod(s, &end);
    while (end && *end && isspace((unsigned char)*end)) end++;
    return end && *end == '\0';
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
        
    }

    fclose(f);
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
        
    }

    fclose(f);
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