#include <stdio.h>
#include <stdlib.h>
#include "probability_distribution.h"

#define LINE_LENGTH 1024

int ProbDist_load(ProbHistogram* h, const char* filename) {
    FILE* f=fopen(filename, "r");
    if (! f) return -1;

    char *buffer = NULL;
    size_t line_length = 0;
    h->type = -1;
    
    int num_tokens = 0;
    int duration = -1;
    int max_duration = -1;
    double probability = -1;
    int type = -1;
    
    //Find the longest duration
    while (getline(&buffer, &line_length, f) > 0) {
        sscanf(buffer, "%d %lf", &duration, &probability);
        if (duration > max_duration) max_duration = duration;
    }
    
    rewind(f);
    
    h->max_duration = max_duration;
    h->CPUprobs = (double*) malloc((h->max_duration + 1) * sizeof(double));
    h->IOprobs = (double*) malloc((h->max_duration + 1) * sizeof(double));
        
    while (getline(&buffer, &line_length, f) > 0){
        // got line in buf
        
        //Find the ResourceType (CPU=0, IO=1)
        num_tokens = sscanf(buffer, "TYPE %d", &type);
        //printf("%d %d\n", type, num_tokens);
        if (num_tokens == 1) {
            if (type > 1) {
                printf("Found wrong type (CPU or IO) in %s. Please check the file.\n", filename);
                return -1;
            }
            h->type = type;
            
            if (h->type == 0) h->CPUprobs[0] = 1;
            else if (h->type == 1) h->IOprobs[0] = 1;
            
            //printf("Current type: %s\n", h->type==0 ? "CPU" : "IO");
            goto next_round;
        }
        
        //Find the durations and their respective probabilities
        num_tokens = sscanf(buffer, "%d %lf", &duration, &probability);
        if (num_tokens == 2){
            if (duration > 0 && probability <= 1) {
                //printf("%d\n", h->type);
                switch(h->type) {
                    case 0:
                        //printf("CPU\n");
                        h->CPUprobs[duration] = probability;
                        goto next_round;
                    case 1:
                        //printf("IO\n");
                        h->IOprobs[duration] = probability;
                        goto next_round;
                    default:
                        goto next_round;
                }
                goto next_round;
            }
            else {
                printf("Found a probability > 1 in %s. Returning.\n", filename);
                return -1;
            }
            
        }
        next_round:
            //printf("NEXT ROUND!\n");
            //printf("%stokens: %d\n", buffer, num_tokens);
        ;
    }
    if (buffer) free(buffer);
    fclose(f);
    return 1;
}

int ProbDist_save(const ProbHistogram* h, const char* filename){
    FILE* f=fopen(filename, "w");
    if (! f) return -1;
    
    if (h->CPUprobs[0]) {
        fprintf(f, "TYPE %d\n", 0);
        for (int i=1; i<h->max_duration+1; i++) {
            if (h->CPUprobs[i]) fprintf(f, "%d %.2f\n", i, h->CPUprobs[i]);
        }
        fprintf(f, "\n");
    }
    
    if (h->IOprobs[0]) {
        fprintf(f, "TYPE %d\n", 1);
        for (int i=1; i<h->max_duration+1; i++) {
            if (h->IOprobs[i]) fprintf(f, "%d %.2f\n", i, h->IOprobs[i]);
        }
    }
    
    fclose(f);
    return 1;
}

// Checks sum of probabilities for CPU histogram. Returns 1 if it's 1, -1 if less than 1,or more than 1, 0 if the sum is 0
int ProbDist_checkCPU(const ProbHistogram* h) {
    double CPUsum = 0;
    double* CPUprobs = h->CPUprobs;
    
    for (int i=1; i<h->max_duration+1; i++) {
        
        CPUsum += CPUprobs[i];
        //printf("CPUsum = %.2f\n", CPUsum);
        // if sum of probabilities for CPU bursts is bigger than 1, returns -1
        if (CPUsum - 1 > 0) {
            printf("CPUsum = %.2f > 0\n", CPUsum);
            return -1;
        }
    }
    
    // if sum of probabilities for CPU bursts is less than 1 but not zero (uninitialized array), returns -1
    if (CPUsum < 1 && CPUsum != 0) return -1;
    
    // if sum of probabilities for CPU bursts is zero (uninitialized array), returns 0
    if (CPUsum == 0) return 0;
    
    // if sum of probabilities for CPU bursts is 1, returns 1
    return 1;
}

// Checks sum of probabilities for IO histogram. Returns 1 if it's 1, -1 if less than 1,or more than 1, 0 if the sum is 0
int ProbDist_checkIO(const ProbHistogram* h) {
    
    double IOsum = 0;
    double* IOprobs = h->IOprobs;
    
    for (int i=1; i<h->max_duration+1; i++) {
        
        IOsum += IOprobs[i];
        //printf("IOsum = %.2f\n", IOsum);
        // if sum of probabilities for IO bursts is bigger than 1, returns -1
        if (IOsum - 1 > 0) {
            printf("IOsum = %.2f > 0\n", IOsum);
            return -1;
        }
    }
    
    // if sum of probabilities for IO bursts is less than 1 but not zero (uninitialized array), returns -1
    if (IOsum < 1 && IOsum != 0) return -1;
    
    // if sum of probabilities for IO bursts is zero (uninitialized array), returns 0
    if (IOsum == 0) return 0;
    
    // if sum of probabilities for IO bursts is 1, returns 1
    return 1;
}
