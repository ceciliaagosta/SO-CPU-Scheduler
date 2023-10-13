#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "probability_distribution.h"

#define print_array(data, n) _Generic((*data), int: print_int, double: print_double)(data,n)
void print_int (const int* data, size_t n) {
    for(size_t i=0; i<n; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
}
void print_double (const double* data, size_t n) {
    for(size_t i=0; i<n; i++) {
        printf("%.3lf ", data[i]);
    }
    printf("\n");
}


//Function to generate random numbers between 0 and 1, in a uniform distribution
double unif_rand() {
    return (double) rand() / RAND_MAX;
}


//Function to generate n CPU bursts and m IO bursts based on the given ProbHistogram h. Saves them in the given arrays. Uses the cumulative probabilities method
void CDF_generator(ProbHistogram* h, int* CPUres, int* IOres, int n, int m) {
    
    //Check if given probabilities sum up to 1
    int CPUcheck = ProbDist_checkCPU(h);
    int IOcheck = ProbDist_checkIO(h);
    if (CPUcheck < 0 || IOcheck < 0) {
        printf("Sum of probabilities doesn't amount to 1 in either the CPU or the IO histograms.\n");
        printf("Incorrect sum in: %s  %s\n", CPUcheck<0?"CPU":"--", IOcheck<0?"IO":"--");
        return;
    }
    
    //Generate the cumulative probabilities array for CPU
    int max_duration = h->max_duration + 1;
    double random;
    
    if (CPUcheck) {
        double* CPUprobs_sum = (double*) malloc(max_duration * sizeof(double));
        CPUprobs_sum[0] = 0;
        //printf("CPU cumulative probabilities: ");
        for (int i=1; i<max_duration; i++) {
            CPUprobs_sum[i] += CPUprobs_sum[i-1] + h->CPUprobs[i];
            //printf("%.3lf ", CPUprobs_sum[i]);
        }
        //printf("\n");
        //Generate integer and save in right array
        for (int i=0; i<n; i++) {
            random = unif_rand();
            for (int j=1; j<max_duration; j++) {
                if (random <= CPUprobs_sum[j]) {
                    CPUres[i] = j;
                    break;
                }
            }
        }
        free(CPUprobs_sum);
    }
    //Generate the cumulative probabilities array for IO
    if (IOcheck) {
        double* IOprobs_sum = (double*) malloc(max_duration * sizeof(double));
        IOprobs_sum[0] = 0;
        //printf("IO cumulative probabilities: ");
        for (int i=1; i<max_duration; i++) {
            IOprobs_sum[i] += IOprobs_sum[i-1] + h->IOprobs[i];
            //printf("%.3lf ", IOprobs_sum[i]);
        }
        //printf("\n");
        //Generate integer and save in right array
        for (int i=0; i<m; i++) {
            random = unif_rand();
            for (int j=1; j<max_duration; j++) {
                if (random <= IOprobs_sum[j]) {
                    IOres[i] = j;
                    break;
                }
            }
        }
        free(IOprobs_sum);
    }
    
    return;
    
}


void Alias_generator(double* hist, int n, double* acceptance, int* alias) {
    
    //Allocate and initialize the two tables for acceptance and alias
    acceptance = (double*) malloc(sizeof(double) * n);
    alias = (int*) malloc(sizeof(int) * n);
    
    acceptance[0] = 0;
    alias[0] = 0;
    for (int i=1; i<n; i++) {
        acceptance[i] = hist[i] * (double)n;
        printf("Acceptance[%d]: %.3lf ", i, acceptance[i]);
        if (acceptance[i] == 1) alias[i] = i;       //if the acceptance is 1, the alias is useless, can initialize as the index.
        else alias[i] = 0;
        printf("Alias[%d]: %d\n", i, alias[i]);
    }
    
    int overfull, underfull;
    while (1) {
        overfull = -1;
        underfull = -1;
        for (int i=n-1; i>=0; i--) {
            if (acceptance[i] > 1) overfull = i;
            if (acceptance[i] < 1 && alias[i] == 0) underfull = i;
        }
        
        if (overfull == -1 || underfull == -1) break;
        
        printf("Overfull: %d   Underfull: %d\n", overfull, underfull);
        
        alias[underfull] = overfull;
        acceptance[overfull] += acceptance[underfull] - 1;
    }
    
    print_array(acceptance, n);
    print_array(alias, n);
}



int main(int argc, char** argv) {
    
    if (argc<5){
        printf("usage %s <seed> <CPU bursts> <IObursts> <in>\n", argv[0]);
        exit(-1);
    }
    
    //seed the RNG
    if(atoi(argv[1]) == 0) srand(time(NULL));
    else srand(atoi(argv[1]));
    
    //load the histogram from file and prepare useful variables
    ProbHistogram h;
    int load = ProbDist_load(&h, argv[4]);
    if (load < 0) exit(-1);
    printf("read [%s]\n", argv[4]);
    
    int max_duration = h.max_duration + 1;

    int CPUnum = atoi(argv[2]);
    int IOnum = atoi(argv[3]);
    
    int* CPUbursts = (int*) malloc(CPUnum * sizeof(int));
    int* IObursts = (int*) malloc(IOnum * sizeof(int));
    for (int i=0; i<CPUnum; i++) CPUbursts[i] = 0;
    for (int i=0; i<IOnum; i++) IObursts[i] = 0;
    
    
    double* CPUacceptance;
    int* CPUalias;
    
    //call the sampler
    Alias_generator(h.CPUprobs, max_duration, CPUacceptance, CPUalias);
    
    CDF_generator(&h, CPUbursts, IObursts, CPUnum, IOnum);
    
    /*if (ProbDist_checkCPU(&h) == 1) {
        printf("CPU bursts: ");
        print_array(CPUbursts, CPUnum);
    }
    if (ProbDist_checkIO(&h) == 1) {
        printf("IObursts: ");
        print_array(IObursts, IOnum);
    }*/
    
    //compare extraction percentages with original probabilities
    int* CPUcompare = (int*) malloc(max_duration * sizeof(int));
    int* IOcompare = (int*) malloc(max_duration * sizeof(int));
    for (int i=0; i<max_duration; i++) {
        CPUcompare[i] = 0;
        IOcompare[i] = 0;
    }
    
    for (int i=1; i<CPUnum; i++) {
        CPUcompare[CPUbursts[i]]++;
    }
    for (int i=1; i<IOnum; i++) {
        IOcompare[IObursts[i]]++;
    }
    
    //print_array(CPUcompare, max_duration);
    //print_array(IOcompare, max_duration);
    
    for (int i=1; i<max_duration; i++) {
        
        float cpu = (float)CPUcompare[i] / CPUnum;
        float io = (float)IOcompare[i] / IOnum;
        
        printf("Duration: %d --> CPU Percentage: %.3lf || CPU Probability: %.3lf\n", i, cpu, h.CPUprobs[i]);
        printf("                IO Percentage:  %.3lf || IO Probability:  %.3lf\n\n", io, h.IOprobs[i]);
    }
    
    //free allocated memory
    free(CPUbursts);
    free(IObursts);
    free(CPUcompare);
    free(IOcompare);
    
    return 0;
}
