#include "fake_process.h"
#include "linked_list.h"
                        
#pragma once


typedef struct {
    ListItem list;
    int pid;
    ListHead events;
    
    int burst;          //How long the current burst is (does NOT reset if the process is preempted)
    float next_burst;   //Predicted value for the next burst (or next time quantum, depends on scheduler)
    
    int preempted;      //True if the process has been preempted during its previous CPU burst
    int entry_time;     //Time at which the process has entered the ready queue last time (used for calculating average waiting time)
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, int cpu, void* args);

typedef struct FakeOS{
    ListHead ready;
    ListHead waiting;
    int timer;
    ScheduleFn schedule_fn;
    void* schedule_args;

    ListHead processes;
    
    float avg_ArrivalTime;      //Arrival Time accumulator for the processes (used to compute and store average at the end of the simulation)
    float avg_WaitingTime;      //Waiting Time accumulator for the processes (used to compute and store average at the end of the simulation)
    
    int num_cpus;
    float alpha;
    FakePCB** running;
} FakeOS;

void FakeOS_init(FakeOS* os, int num_cpus, float alpha);       //Inizializza il FakeOS os (tutti i campi a 0 e le liste vuote)

void FakeOS_simStep(FakeOS* os, int num_cpus);    //Simula la creazione e scheduling di processi in un SO, avanzando la simulazione un passo alla volta

void FakeOS_destroy(FakeOS* os);    //Vuota. Dovrebbe eliminare le strutture del falso SO (?), ma non è stata scritta
