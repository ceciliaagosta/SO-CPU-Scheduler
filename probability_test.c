#include <stdio.h>
#include <stdlib.h>
#include "probability_distribution.h"

int main(int argc, char** argv) {
    if (argc<3){
        printf("usage %s <in> <out>\n", argv[0]);
        exit(-1);
    }
    ProbHistogram h;
    int load = ProbDist_load(&h, argv[1]);
    if (load < 0) exit(-1);
    printf("read [%s]\n", argv[1]);
    //printf("type: %d\n", h.type);
    for (int i=1; i<=h.max_duration; i++) {
        printf("CPU Duration: %d, Probability: %.2f\n", i, h.CPUprobs[i]);
        printf("IO  Duration: %d, Probability: %.2f\n", i, h.IOprobs[i]);
    }
    int save = ProbDist_save(&h, argv[2]);
    printf("saved [%s]\n", argv[2]);
    if (save < 0) {
        printf("File was not saved.\n");
        exit(-1);
    }
    
    int checkCPU = ProbDist_checkCPU(&h);
    int checkIO = ProbDist_checkIO(&h);
    printf("Probability check for CPU: %d\n", checkCPU);
    printf("Probability check for IO: %d\n", checkIO);
    
    free(h.CPUprobs);
    free(h.IOprobs);
    
    return 0;
}
