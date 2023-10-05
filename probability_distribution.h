#pragma once

#include "fake_process.h"

// Histogram with duration probabilities. The index of 'probs' is the duration and its value is the probability of having an event with that duration
typedef struct {
    int type;               // CPU=0; IO=1
    int max_duration;
    float* CPUprobs;       // Histogram of CPU burst durations' probabilities
    float* IOprobs;        // Histogram of IO burst durations' probabilities
} ProbHistogram;

int ProbDist_load(ProbHistogram* h, const char* filename);  //Loads the ProbHistogram h with info taken from the File 'filename' and returns the                                                                                  ResourceType
