#include <stdio.h>
#include "mode.h"
#include "nodesAndEdges.h"
#include "csvParse.h"
#include "problem1.h"
#include "problem2.h"
#include "problem3.h"
#include "problem4.h"
#include "problem5.h"
#include "problem6.h"

int main() {

    parseRoadmapCSV("Roadmap-Dhaka.csv");
    parseMetroCSV("Routemap-DhakaMetroRail.csv");
    parseBusCSV("Routemap-BikolpoBus.csv", MODE_BIKOLPO);
    parseBusCSV("Routemap-UttaraBus.csv", MODE_UTTARA);

    while (1) 
    {
        printf("\n-------Mr Efficient--------\n");
        printf("[1] Shortest Car Route [Problem 1]\n");
        printf("[2] Cheapest Route(Car and Metro) [Problem 2]\n");
        printf("[3] Cheapest Route(Car, Metro and Bus) [Problem 3]\n");
        printf("[4] Cheapest Route with Time Schedule [Problem 4]\n");
        printf("[5] Fastest Route with Time Schedule [Problem 5]\n");
        printf("[6] Cheapest Route with Deadline [Problem 6]\n");
        printf("[7] Quit\n");
        printf("-----------------------------\n");

        int choice;
        printf("Enter Choice: ");
        scanf("%d", &choice);
        printf("-----------------------------\n");

        if (choice == 7) {
            break;
        }

        switch (choice) 
        {
        case 1:
            runProblem1();
            break;
        case 2:
            runProblem2();
            break;
        case 3:
            runProblem3();
            break;
        case 4:
            runProblem4();
            break;
        case 5:
            runProblem5();
            break;
        case 6:
            runProblem6();
            break;
        default:
            printf("Invalid choice\n");
            break;
        }
        
    }
    
    return 0;
}