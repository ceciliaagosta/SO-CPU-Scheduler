#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
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

//Function to compare percentages of duration of bursts with original probabilities from histogram h
void compare_percentages(ProbHistogram* h, int* CPUbursts, int* IObursts, int max_duration, int CPUnum, int IOnum) {
    
    int* CPUcompare = (int*) malloc(max_duration * sizeof(int));
    int* IOcompare = (int*) malloc(max_duration * sizeof(int));
    for (int i=0; i<max_duration; i++) {
        CPUcompare[i] = 0;
        IOcompare[i] = 0;
    }
    
    for (int i=0; i<CPUnum; i++) {
        //printf("CPUbursts[%d]=%d\n", i, CPUbursts[i]);
        CPUcompare[CPUbursts[i]]++;
    }
    for (int i=0; i<IOnum; i++) {
        IOcompare[IObursts[i]]++;
        //printf("IObursts[%d]=%d\n", i, IObursts[i]);
    }
    
    //print_array(CPUcompare, max_duration);
    //print_array(IOcompare, max_duration);
    
    for (int i=1; i<max_duration; i++) {
        
        float cpu = (float)CPUcompare[i] / CPUnum;
        float io = (float)IOcompare[i] / IOnum;
        
        printf("Duration: %d --> CPU Percentage: %.3lf || CPU Probability: %.3lf\n", i, cpu, h->CPUprobs[i]);
        printf("                IO Percentage:  %.3lf || IO Probability:  %.3lf\n\n", io, h->IOprobs[i]);
    }
    
    //free allocated memory
    free(CPUcompare);
    free(IOcompare);
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

//Function to generate alias and acceptance table for Alias Method
void Alias_generator(double* hist, int n, double* acceptance, int* alias) {
    
    //Initialize the two tables for acceptance and alias
    acceptance[0] = 0;
    alias[0] = 0;
    for (int i=1; i<n; i++) {
        acceptance[i] = hist[i] * (double)n;
        //printf("Acceptance[%d]: %.3lf ", i, acceptance[i]);
        if (acceptance[i] == 1) alias[i] = i;       //if the acceptance is 1, the alias is useless, can initialize as the index.
        else alias[i] = 0;
        //printf("Alias[%d]: %d\n", i, alias[i]);
    }
    //printf("\n");
    
    int overfull, underfull;
    while (1) {
        overfull = -1;
        underfull = -1;
        for (int i=n-1; i>=0; i--) {
            if (acceptance[i] > 1) overfull = i;
            if (acceptance[i] < 1 && alias[i] == 0) underfull = i;
        }
        
        if (overfull == -1 || underfull == -1) break;
        
        //printf("Overfull: %d   Underfull: %d\n", overfull, underfull);
        
        alias[underfull] = overfull;
        acceptance[overfull] += acceptance[underfull] - 1;
    }
    
    /*printf("Acceptance and Alias arrays: \n");
    print_array(acceptance, n);
    print_array(alias, n);
    printf("\n");*/
}

//Function to sample from tables in Alias Method
void Alias_sampler(double* acceptance, int* alias, int* res, int n, int samples) {
    
    for (int s=0; s<samples; s++) {
        double random = unif_rand();
        //printf("random: %.3lf  ", random);
        
        int i = (int)floor(n * random) + 1;
        //printf("i: %d  ", i);
        double y = n * random + 1 - i;
        //printf("y: %.3lf  ", y);
        
        if (y <= acceptance[i]) res[s] = i;
        else res[s] = alias[i];
        
        //printf("res[%d]: %d\n", s, res[s]);
    }
}

//Alias Method
int* Alias_function(double* hist, int n, int samples) {
    
    double* acceptance = (double*) malloc(sizeof(double) * n);
    int* alias = (int*) malloc(sizeof(int) * n);
    
    Alias_generator(hist, n, acceptance, alias);
    
    int* res = (int*) malloc(samples * sizeof(int));
    Alias_sampler(acceptance, alias, res, n-1, samples);
    
    free(acceptance);
    free(alias);
    
    return res;
}


//Function to write process traces in a file
int save_trace(const int* CPUtrace, const int* IOtrace, int n, int id, const char* filename){
    FILE* f=fopen(filename, "w");
    if (! f) return -1;
    
    int arrival = floor(unif_rand() * 10);
    fprintf(f, "PROCESS \t%d %d\n", id, arrival);
    
    for (int i=0; i<n; i++) {
        
        if (CPUtrace[i]) {
            fprintf(f, "CPU_BURST \t%d\n", CPUtrace[i]);
        }
        
        if (IOtrace[i]) {
            fprintf(f, "IO_BURST \t%d\n", IOtrace[i]);
        }
    }
    
    if (CPUtrace[n]) {
        fprintf(f, "CPU_BURST \t%d\n", CPUtrace[n]);
    }
    
    fclose(f);
    return 1;
}


int main(int argc, char** argv) {
    
    if (argc<5){
        printf("usage %s <seed> <CPU/IO bursts> <save> <in>\n", argv[0]);
        exit(-1);
    }
    
    //seed the RNG
    if(atoi(argv[1]) == 0) srand(time(NULL));
    else srand(atoi(argv[1]));
    
    int CPUnum = atoi(argv[2]) + 1;
    int IOnum = atoi(argv[2]);
    
    int save = atoi(argv[3]);
    
    ProbHistogram h;
    
    for (int j=4; j<argc; j++) {
        
        //load the histogram from file and prepare useful variables
        int load = ProbDist_load(&h, argv[j]);
        if (load < 0) exit(-1);
        printf("read [%s]\n", argv[j]);
        
        int max_duration = h.max_duration + 1;
        
        int* CPUbursts = (int*) malloc(CPUnum * sizeof(int));
        int* IObursts = (int*) malloc(IOnum * sizeof(int));
        for (int i=0; i<CPUnum; i++) CPUbursts[i] = 0;
        for (int i=0; i<IOnum; i++) IObursts[i] = 0;
        
        //call the sampler
        
        int* CPUres = Alias_function(h.CPUprobs, max_duration, CPUnum);
        int* IOres = Alias_function(h.IOprobs, max_duration, IOnum);
        
        CDF_generator(&h, CPUbursts, IObursts, CPUnum, IOnum);
        
        /*if (ProbDist_checkCPU(&h) == 1) {
            printf("CPU bursts: ");
            print_array(CPUbursts, CPUnum);
        }
        if (ProbDist_checkIO(&h) == 1) {
            printf("IObursts: ");
            print_array(IObursts, IOnum);
        }
        printf("\nCPUres:  ");
        print_array(CPUres, CPUnum);
        printf("IOres: ");
        print_array(IOres, IOnum);*/
        
        //compare extraction percentages with original probabilities
        printf("Comparison for CDF Method: \n");
        compare_percentages(&h, CPUbursts, IObursts, max_duration, CPUnum, IOnum);
        printf("\nComparison for Alias Method: \n");
        compare_percentages(&h, CPUres, IOres, max_duration, CPUnum, IOnum);
        
        //save trace files
        if (save) {
            char filename_cdf[sizeof "cdf1.txt"];
            sprintf(filename_cdf, "cdf%d.txt", j-3);
            int cdf_trace = save_trace(CPUbursts, IObursts, IOnum, j-3, filename_cdf);
            printf("Saved cdf trace %d\n\n", j-3);
            
            if (cdf_trace < 0) {
                printf("Failed to save cdf trace %d. Exiting...\n\n", j-3);
                exit(-1);
            }
            
            char filename_alias[sizeof "alias1.txt"];
            sprintf(filename_alias, "alias%d.txt", j-3);
            int alias_trace = save_trace(CPUbursts, IObursts, IOnum, j-3, filename_alias);
            printf("Saved alias trace %d\n\n", j-3);
            
            if (alias_trace < 0) {
                printf("Failed to save alias trace %d. Exiting...\n\n", j-3);
                exit(-1);
            }
        }
        
        //free allocated memory
        free(CPUbursts);
        free(IObursts);
        free(h.CPUprobs);
        free(h.IOprobs);
        free(CPUres);
        free(IOres);
    }
    
    return 0;
}
