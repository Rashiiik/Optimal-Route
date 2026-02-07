#include <stdio.h>

#define MAX_NODES 10000
#define INF 9999999999

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

Node nodes[] = {
    {0, "Home", 23.7806, 90.4070},
    {1, "Bus Stop", 23.7810, 90.4080},
    {2, "Metro Station", 23.7820, 90.4100},
    {3, "Office", 23.7850, 90.4120},
    {4, "Market", 23.7830, 90.4090},
    {5, "Hospital", 23.7840, 90.4110}
};

Edge edges[] = {                    // Sample
    {0, 1, MODE_CAR, 2.0, 40, 30},  // from Home to Bus Stop
    {0, 4, MODE_CAR, 3.0, 60, 30},  // from Home to Market
    {4, 3, MODE_CAR, 2.5, 50, 30},  // Market to Office
    {1, 3, MODE_CAR, 5.0, 100, 30}  // Bus Stop to Office
};

int numEdges = sizeof(edges)/sizeof(edges[0]);

void printDetails(int path[], int pathLen, int source, int target) {

    double carRate = 20.0;                     // Its never this low

    printf("Source: (%f, %f)\n", nodes[source].lon, nodes[source].lat);
    printf("Destination: (%f, %f)\n", nodes[target].lon, nodes[target].lat);

    printf("Starting Time at source: 6:45 am\n");
    printf("Destination reaching time: 8:30 am\n");

    for(int i=pathLen-1;i>0;i--)
    {
        int from = path[i];
        int to = path[i-1];

        double distSeg = 0;                   // Finding Edge :)

        for(int j=0;j<numEdges;j++)
        {
            if(edges[j].from == from && edges[j].to == to && edges[j].mode == MODE_CAR)
            {
                distSeg = edges[j].distance;
                break;
            }
        }

        double costSeg = distSeg * carRate;

        printf("Ride Car from %s (%.6f, %.6f) to %s (%.6f, %.6f), Distance: %.2f km, Cost: ৳%.2f\n", nodes[from].name, nodes[from].lat, nodes[from].lon, nodes[to].name, nodes[to].lat, nodes[to].lon, distSeg, costSeg);
    }
}

void runProblem1() {
    
    int source = 0;
    int target = 3;

    int numNodes = sizeof(nodes)/sizeof(nodes[0]);
    
    for (int i = 0; i < numNodes; i++)
    {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }

    dist[source] = 0;
    
    for(int count=0; count<numNodes; count++)               // Dijkstra go brrrrrrrrrr
    {
        int u = -1;
        double minDist = INF;

        for(int i=0;i<numNodes;i++)
        {
            if(!visited[i] && dist[i] < minDist)
            {
                minDist = dist[i];
                u = i;
            }
        }

        if(u == -1) 
        {
            break;     // Cannot reach the node (T-T)
        } 

        visited[u] = 1;              // We do sum relaxing
            
        for(int i=0;i<numEdges;i++)
        {
            if(edges[i].from == u && edges[i].mode == MODE_CAR)
            {
                int v = edges[i].to;

                if(dist[v] > dist[u] + edges[i].distance)
                {
                    dist[v] = dist[u] + edges[i].distance;
                    prev[v] = u;
                }
            }
        }
    }

    int path[numNodes];
    int countPath = 0;

    for(int at=target; at!=-1; at=prev[at])
    {
        path[countPath++] = at;
    }

    /*printf("Shortest Car Route from %s to %s:\n", nodes[source].name, nodes[target].name);

    for(int i=countPath-1;i>=0;i--)
    {
        printf("%s", nodes[path[i]].name);
        if(i != 0) 
        {
            printf(" -> ");
        }
    }
    printf("\nTotal distance: %.2f km\n", dist[target]);*/

    printDetails(path, countPath, source, target);
    
}



int main() {

    while (1)
    {
        printf("\n-------Mr Efficient--------\n");
        printf("[1] Shortest Car Route\n");
        printf("[7] Quit\n");
        printf("-----------------------------\n");

        int choice;
        printf("Enter Choice: ");

        scanf("%d", &choice);

        printf("-----------------------------\n");

        if (choice == 7)
        {
            break;
        }

        switch (choice)
        {
        case 1:
            printf("Problem No: %d\n", choice);
            runProblem1();
            break;
        
        default:
            break;
        
        }
        
    }
    
}