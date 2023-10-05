#include "probability_distribution.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc<2){
        printf("usage %s <in>\n", argv[0]);
        exit(-1);
    }
    ProbHistogram h;
    int load = ProbDist_load(&h, argv[1]);
    if (load < 0) exit(-1);
    printf("read [%s]\n", argv[1]);
    //printf("type: %d\n", h.type);
    for (int i=0; i<=h.max_duration; i++) {
        printf("CPU Duration: %d, Probability: %.2f\n", i, h.CPUprobs[i]);
        printf("IO  Duration: %d, Probability: %.2f\n", i, h.IOprobs[i]);
    }
    return 0;
}