#include "fake_process.h"
#include "linked_list.h"

#ifndef NUM_CPUS
#define NUM_CPUS 4      //Number of CPUs available to the OS
#endif

#ifndef ALPHA
#define ALPHA 0.5       //Value of alpha in burst/quantum prediction formula (i.e. Exponential Average)
#endif
                        
#pragma once


typedef struct {
    ListItem list;
    int pid;
    ListHead events;
    
    int burst;          //How long the current burst is (does NOT reset if the process is preempted)
    float next_burst;   //Predicted value for the next burst (or next time quantum, depends on scheduler)
    
    int preempted;      //True if the process has been preempted during its previous CPU burst
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, int cpu, void* args);

typedef struct FakeOS{
    FakePCB* running[NUM_CPUS];
    ListHead ready;
    ListHead waiting;
    int timer;
    ScheduleFn schedule_fn;
    void* schedule_args;

    ListHead processes;
    
    float avg_ArrivalTime;  //Arrival Time accumulator for the processes (used to compute and store average at the end of the simulation)
} FakeOS;

void FakeOS_init(FakeOS* os);       //Inizializza il FakeOS os (tutti i campi a 0 e le liste vuote)

void FakeOS_simStep(FakeOS* os);    //Simula la creazione e scheduling di processi in un SO, avanzando la simulazione un passo alla volta

void FakeOS_destroy(FakeOS* os);    //Vuota. Dovrebbe eliminare le strutture del falso SO (?), ma non è stata scritta
