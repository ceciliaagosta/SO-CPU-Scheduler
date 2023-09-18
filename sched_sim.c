#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
    int quantum;
} SchedRRArgs;

//***** Round Robin scheduler with Quantum based preemption *****
void schedRR(FakeOS* os, int cpu, void* args_){
    
    if (os->running[cpu]) return;
    
    SchedRRArgs* args=(SchedRRArgs*)args_;

        // look for the first process in ready
        // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }

    FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
    os->running[cpu]=pcb;
    printf("\n\t\tTaking the first ready process: %d\n", pcb->pid);
        
    assert(pcb->events.first);
    ProcessEvent* e = (ProcessEvent*)pcb->events.first;
    assert(e->type==CPU);
    
    if (!pcb->preempted) {
        os->avg_ArrivalTime += os->timer;
        printf("\t\t*** TOTAL ARRIVAL TIME until now: %.2f ***\n\n", os->avg_ArrivalTime);
    }
    
    pcb->preempted = 0;
        
    // look at the first event
    // if duration>quantum
    // push front in the list of event a CPU event of duration quantum
    // alter the duration of the old event subtracting quantum
    if (e->duration>args->quantum) {
        pcb->preempted = 1;
        ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
        qe->list.prev=qe->list.next=0;
        qe->type=CPU;
        qe->duration=args->quantum;
        e->duration-=args->quantum;
        List_pushFront(&pcb->events, (ListItem*)qe);
    }
};


//Function for finding the Shortest Job in SJF scheduling algorithms. prediction must be 0 for algorithms with no burst/quantum prediction
FakePCB* Sched_findShortestJob(FakeOS* os, int prediction) {
    
    FakePCB* shortest_process = (FakePCB*) os->ready.first;
    assert(shortest_process->events.first);
    ProcessEvent* short_e = (ProcessEvent*)shortest_process->events.first;
    assert(short_e->type==CPU);
    
    if (prediction) {
        //take the first process in ready as the shortest job and check that it is going to CPU
        float shortest_burst = shortest_process->next_burst;
        
        printf("Ready queue: [pid=%d duration=%d pred=%.2f]", shortest_process->pid, short_e->duration, shortest_burst);
        
        //now we look for the actual shortest process through the whole ready queue, based on our next burst prediction
        ListItem* aux = os->ready.first->next;
        while (aux){
            FakePCB* current_process = (FakePCB*) aux;
            
            assert(current_process->events.first);
            ProcessEvent* curr_e = (ProcessEvent*)current_process->events.first;
            float current_burst = current_process->next_burst;
            
            printf(" [pid=%d duration=%d pred=%.2f]", current_process->pid, curr_e->duration, current_burst);
            
            if (curr_e->type == CPU) {
                if (current_burst < shortest_burst) {
                    shortest_burst = current_burst;
                    shortest_process = current_process;
                }
            }
            
            aux = aux->next;
        }
        printf("\n");
    }
    else {
        int short_duration = short_e->duration;
        
        printf("Ready queue: [pid=%d duration=%d]", shortest_process->pid, short_e->duration);
        
        //now we look for the actual shortest process through the whole ready queue
        ListItem* aux = os->ready.first->next;
        while (aux){
            FakePCB* current_process = (FakePCB*) aux;
            
            assert(current_process->events.first);
            ProcessEvent* curr_e = (ProcessEvent*)current_process->events.first;
            
            printf(" [pid=%d duration=%d]", current_process->pid, curr_e->duration);
            
            if (curr_e->type == CPU) {
                if (curr_e->duration < short_duration) {
                    short_duration = curr_e->duration;
                    shortest_process = current_process;
                }
            }
            
            aux = aux->next;
        }
        printf("\n");
    }
    
    return shortest_process;
}


//***** Non-preemptive Shortest Job First Scheduler *****
void schedSJF_np(FakeOS* os, int cpu, void* args_){
    
    if (os->running[cpu]) return;

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = Sched_findShortestJob(os, 0);
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
    printf("\n\t\tTaking the shortest ready process: %d\n", shortest_process->pid);
    
    os->avg_ArrivalTime += os->timer;
    printf("\t\t*** TOTAL ARRIVAL TIME until now: %.2f ***\n\n", os->avg_ArrivalTime);
};


//***** Preemptive Shortest Job First Scheduler (Shortest Remaining Job First) *****
void schedSRJF(FakeOS* os, int cpu, void* args_){

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = Sched_findShortestJob(os, 0);
    ProcessEvent* short_e = (ProcessEvent*)shortest_process->events.first;
    
    //check if something is running on the current CPU
    if (os->running[cpu]) {
        ProcessEvent* running_e = (ProcessEvent*) os->running[cpu]->events.first;
        //if the process running is shorter than the shortest process in ready, there is no need to switch
        if (running_e->duration <= short_e->duration) return;
        
        printf("\t\tPreempting the current running process: [pid=%d duration=%d]", os->running[cpu]->pid, running_e->duration);
        os->running[cpu]->preempted = 1;
        List_pushFront(&os->ready, (ListItem*) os->running[cpu]);
    }
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
    printf("\n\t\tTaking the shortest ready process: %d\n", shortest_process->pid);
    
    if (!shortest_process->preempted) {
        os->avg_ArrivalTime += os->timer;
        printf("\t\t*** TOTAL ARRIVAL TIME until now: %.2f ***\n\n", os->avg_ArrivalTime);
    }
    
    shortest_process->preempted = 0;
};


//***** Preemptive Shortest Job First Scheduler w/Burst Prediction for the next burst *****
void schedSJF_BP(FakeOS* os, int cpu, void* args_){

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = Sched_findShortestJob(os, 1);
    float shortest_burst = shortest_process->next_burst;
    
    //check if something is running on the current CPU
    if (os->running[cpu]) {
        ProcessEvent* running_e = (ProcessEvent*) os->running[cpu]->events.first;
        //if the process running is shorter than the shortest process in ready, there is no need to switch
        if (os->running[cpu]->next_burst <= shortest_burst) return;
        
        os->running[cpu]->next_burst = os->running[cpu]->next_burst * (1-ALPHA) + os->running[cpu]->burst * ALPHA;

        printf("\t\tPreempting the current running process: [pid=%d duration=%d pred=%.2f]", os->running[cpu]->pid, running_e->duration, os->running[cpu]->next_burst);
        os->running[cpu]->preempted = 1;
        List_pushFront(&os->ready, (ListItem*) os->running[cpu]);
    }
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
    printf("\n\t\tTaking the shortest (predicted) ready process: %d\n", shortest_process->pid);
    
    if (!shortest_process->preempted) {
        os->avg_ArrivalTime += os->timer;
        printf("\t\t*** TOTAL ARRIVAL TIME until now: %.2f ***\n\n", os->avg_ArrivalTime);
    }
    
    shortest_process->preempted = 0;
};


//***** Preemptive Shortest Job First Scheduler w/Quantum Prediction for the next given CPU time quantum *****
void schedSJF_QP(FakeOS* os, int cpu, void* args_){
    
    if (os->running[cpu]) return;

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = Sched_findShortestJob(os, 1);
    ProcessEvent* short_e = (ProcessEvent*)shortest_process->events.first;
    
    if (!shortest_process->preempted) {
        printf("\n\t\tTaking the shortest (predicted) ready process: %d\n", shortest_process->pid);
        os->avg_ArrivalTime += os->timer;
        printf("\t\t*** TOTAL ARRIVAL TIME until now: %.2f ***\n\n", os->avg_ArrivalTime);
    }
    else {
        printf("\t\tResuming a previously preempted process: [pid=%d duration=%d pred=%.2f]\n", shortest_process->pid, short_e->duration, shortest_process->next_burst);
    }
    
    shortest_process->preempted = 0;
    
    short_e = (ProcessEvent*)shortest_process->events.first;
    // look at the first event
    // if duration>quantum
    // push front in the list of event a CPU event of duration quantum
    // alter the duration of the old event subtracting quantum
    if (short_e->duration > round(shortest_process->next_burst)) {
        shortest_process->preempted = 1;
        ProcessEvent* qe = (ProcessEvent*)malloc(sizeof(ProcessEvent));
        qe->list.prev = qe->list.next=0;
        qe->type = CPU;
        qe->duration = round(shortest_process->next_burst);
        short_e->duration -= round(shortest_process->next_burst);
        List_pushFront(&shortest_process->events, (ListItem*)qe);
    }
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
};


//****************************************************************************
//******************************* M A I N ************************************
//****************************************************************************

int main(int argc, char** argv) {
    FakeOS_init(&os);
    SchedRRArgs srr_args;
    srr_args.quantum=5;
    os.schedule_args=&srr_args;
    os.schedule_fn=schedSJF_QP;
  
    for (int i=1; i<argc; ++i){
        FakeProcess new_process;
        int num_events=FakeProcess_load(&new_process, argv[i]);
        printf("loading [%s], pid: %d, events: %d\n",
               argv[i], new_process.pid, num_events);
        if (num_events) {
            FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
            *new_process_ptr=new_process;
            List_pushBack(&os.processes, (ListItem*)new_process_ptr);
        }
    }
    printf("num processes in queue %d\n", os.processes.size);
    
    int total_processes = os.processes.size;
    
    while(1){
        int running_cpus = 0;
        for (int cpu=0; cpu<NUM_CPUS; ++cpu) {
            if (os.running[cpu]) {
                running_cpus = 1;
                break;
            }
        }
        if (!running_cpus && !os.ready.first && !os.waiting.first && !os.processes.first) break;
        
        FakeOS_simStep(&os);
        
        //for debugging
        if (os.timer > 200) break;
    }
    
    if (os.avg_ArrivalTime > 0 && total_processes > 0) {
        printf("\n\n\tTotal arrival time: %.2f", os.avg_ArrivalTime);
        os.avg_ArrivalTime /= total_processes;
        
        printf("\n\n\t*** AVERAGE ARRIVAL TIME (for %d processes): %.2f ***\n\n", total_processes, os.avg_ArrivalTime);
    }
}
