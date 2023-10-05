#include "probability_distribution.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc<2){
        printf("usage %s <in>\n", argv[0]);
        exit(-1);
    }
    ProbHistogram h;
    int type = ProbDist_load(&h, argv[1]);
    if (type < 0) return -1;
    printf("read [%s], type: %s\n",argv[1], type==0 ? "CPU" : "IO");
    //printf("type: %d\n", h.type);
    for (int i=0; i<=h.max_duration; i++) {
        printf("Duration: %d, Probability: %.2f\n", i, h.probs[i]);
    }
    return 0;
}
