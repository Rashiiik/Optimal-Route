#ifndef mode_H
#define mode_H

#define EARTH_RADIUS_KM 6371.0
#define PI 3.14159265358979323846
#define WALK_SPEED_KMH 2.0
#define MAX_WALK_DISTANCE_KM 0.1  
#define VEHICLE_SPEED_PROBLEM5_KMH 10.0
#define VEHICLE_SPEED_KMH 30.0
#define SCHEDULE_INTERVAL_MIN 15  // Every 15 minutes
#define SCHEDULE_START_MIN (6 * 60)  // 6 AM in minutes
#define SCHEDULE_END_MIN (23 * 60)   // 11 PM in minutes

#define CAR_SPEED_PROBLEM6_KMH 20.0
#define METRO_SPEED_PROBLEM6_KMH 15.0
#define BIKOLPO_SPEED_PROBLEM6_KMH 10.0
#define UTTARA_SPEED_PROBLEM6_KMH 12.0

// Add new schedule constants for Problem 6
#define BIKOLPO_INTERVAL_MIN 20
#define BIKOLPO_START_MIN (7 * 60)    // 7 AM
#define BIKOLPO_END_MIN (22 * 60)     // 10 PM

#define UTTARA_INTERVAL_MIN 10
#define UTTARA_START_MIN (6 * 60)     // 6 AM
#define UTTARA_END_MIN (23 * 60)      // 11 PM

#define METRO_INTERVAL_MIN 5
#define METRO_START_MIN (1 * 60)      // 1 AM
#define METRO_END_MIN (23 * 60)       // 11 PM

typedef enum
{
    MODE_WALK,
    MODE_METRO,
    MODE_CAR,
    MODE_BIKOLPO,
    MODE_UTTARA
} Mode;

const char* getModeName(Mode mode);

#endif