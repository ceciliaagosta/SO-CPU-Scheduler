#pragma once

#include "fake_process.h"

// Histogram with duration probabilities. The index of 'probs' is the duration and its value is the probability of having an event with that duration
typedef struct {
    int type;               // CPU=0; IO=1
    int max_duration;       // Max duration registered for the file (must be +1 in array cycles because slot 0 does not count)
    double* CPUprobs;        // Histogram of CPU burst durations' probabilities
    double* IOprobs;         // Histogram of IO burst durations' probabilities
} ProbHistogram;

int ProbDist_load(ProbHistogram* h, const char* filename);  //Loads the ProbHistogram h with info taken from the File 'filename' and returns the                                                                                  ResourceType; returns 1 if everything went well, -1 if there was an error.

int ProbDist_save(const ProbHistogram* h, const char* filename);  //Saves the probability distributions in h to a File 'filename'; returns 1 if everything went well,                                                                   -1 if there was an error.

int ProbDist_checkCPU(const ProbHistogram* h);                 //Checks whether the sum of probabilities is equal to 1 and returns 1 if true; -1 if < 1 or > 1, 0 if 0.
int ProbDist_checkIO(const ProbHistogram* h);
