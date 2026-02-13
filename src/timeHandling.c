#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mode.h"
#include "nodesAndEdges.h"

double arrivalTime[MAX_NODES];

int parseTime(const char* timeStr) {
    // Parse time in format "HH:MM AM/PM" or "H:MM AM/PM"
    int hour, minute;
    char period[3];
    
    if (sscanf(timeStr, "%d:%d %2s", &hour, &minute, period) != 3) 
    {
        printf("Invalid time format. Use format like '5:43 PM' or '9:30 AM'\n");
        return -1;
    }
    
    // Convert to 24-hour format
    if (strcmp(period, "PM") == 0 || strcmp(period, "pm") == 0) {
        if (hour != 12) hour += 12;
    } else if (strcmp(period, "AM") == 0 || strcmp(period, "am") == 0) {
        if (hour == 12) hour = 0;
    } else {
        printf("Invalid period. Use 'AM' or 'PM'\n");
        return -1;
    }
    
    // Convert to minutes from midnight
    int totalMinutes = hour * 60 + minute;
    
    if (totalMinutes < 0 || totalMinutes >= 24 * 60) {
        printf("Invalid time\n");
        return -1;
    }
    
    return totalMinutes;
}

// Add function to format time from minutes
void formatTime(int minutes, char* buffer, int bufferSize) {
    int hour = (minutes / 60) % 24;
    int min = minutes % 60;
    int displayHour = hour;
    const char* period = "AM";
    
    if (hour >= 12) {
        period = "PM";
        if (hour > 12) displayHour = hour - 12;
    }
    if (displayHour == 0) displayHour = 12;
    
    snprintf(buffer, bufferSize, "%d:%02d %s", displayHour, min, period);
}

// Add function to calculate waiting time for next scheduled service
double getWaitingTime(int currentTimeMin, Mode mode) {
    // Cars don't need to wait
    if (mode == MODE_CAR || mode == MODE_WALK) {
        return 0.0;
    }
    
    // Check if service is running
    if (currentTimeMin < SCHEDULE_START_MIN || currentTimeMin >= SCHEDULE_END_MIN) {
        // Service not available
        return INF;
    }
    
    // Calculate minutes since service started today
    int minutesSinceStart = currentTimeMin - SCHEDULE_START_MIN;
    
    // Calculate time to next departure
    int minutesToNext = SCHEDULE_INTERVAL_MIN - (minutesSinceStart % SCHEDULE_INTERVAL_MIN);
    if (minutesToNext == SCHEDULE_INTERVAL_MIN) {
        minutesToNext = 0;  // Service available now
    }
    
    return (double)minutesToNext;
}