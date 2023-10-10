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
    float probability = -1;
    int type = -1;
    
    //Find the longest duration
    while (getline(&buffer, &line_length, f) > 0) {
        sscanf(buffer, "%d %f", &duration, &probability);
        if (duration > max_duration) max_duration = duration;
    }
    fclose(f);
    f=fopen(filename, "r");
    
    h->max_duration = max_duration;
    h->CPUprobs = (float*) malloc((h->max_duration + 1) * sizeof(float));
    h->IOprobs = (float*) malloc((h->max_duration + 1) * sizeof(float));
        
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
        num_tokens = sscanf(buffer, "%d %f", &duration, &probability);
        if (num_tokens == 2){
            if (probability <= 1) {
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
        printf("\n");
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
  
