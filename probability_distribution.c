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
    h->probs = (float*) malloc((h->max_duration + 1) * sizeof(float));
        
    while (getline(&buffer, &line_length, f) > 0){
        // got line in buf
        
        //Find the ResourceType (CPU=0, IO=1)
        num_tokens = sscanf(buffer, "TYPE %d", &type);
        //printf("%d %d\n", type, num_tokens);
        if (num_tokens == 1 && h->type < 0) {
            h->type = type;
            goto next_round;
        }
        
        //Find the durations and their respective probabilities
        num_tokens = sscanf(buffer, "%d %f", &duration, &probability);
        if (num_tokens == 2){
            if (probability <= 1) {
                h->probs[duration] = probability;
                goto next_round;
            }
            else {
                printf("Found a probability > 1 in %s. Returning.\n", filename);
                return -1;
            }
            
        }
        next_round:
            //printf("%stokens: %d\n", buffer, num_tokens);
        ;
    }
    if (buffer) free(buffer);
    fclose(f);
    return h->type;
}
