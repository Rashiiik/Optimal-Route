#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include "mode.h"

#define MAX_LINE 200000
#define MAX_TOKENS 5000

void trim_in_place(char *s);
int split_csv(char *line, char **tokens, int maxTokens);
int is_number_token(const char *s);
void parseRoadmapCSV(const char *filename);
void parseMetroCSV(const char *filename);
void parseBusCSV(const char *filename, Mode busMode);
void exportPathToKML(int path[], int pathLen, const char *filename);

#endif