#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
    int quantum;
} SchedRRArgs;

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
        
    // look at the first event
    // if duration>quantum
    // push front in the list of event a CPU event of duration quantum
    // alter the duration of the old event subtracting quantum
    if (e->duration>args->quantum) {
        ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
        qe->list.prev=qe->list.next=0;
        qe->type=CPU;
        qe->duration=args->quantum;
        e->duration-=args->quantum;
        List_pushFront(&pcb->events, (ListItem*)qe);
    }
};


//Non-preemptive Shortest Job First Scheduler
void schedSJF_np(FakeOS* os, int cpu, void* args_){

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = (FakePCB*) os->ready.first;
    assert(shortest_process->events.first);
    ProcessEvent* short_e = (ProcessEvent*)shortest_process->events.first;
    assert(short_e->type==CPU);
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
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
    printf("\n\t\tTaking the shortest ready process: %d\n", shortest_process->pid);
};


//Classic Preemptive Shortest Job First Scheduler
void schedSJF(FakeOS* os, int cpu, void* args_){

    // look for the first process in ready
    // if none, return
    if (! os->ready.first) {
        printf("\n\t\tNo process found in ready!\n");
        return;
    }
    
    //take the first process in ready as the shortest job and check that it is going to CPU
    FakePCB* shortest_process = (FakePCB*) os->ready.first;
    assert(shortest_process->events.first);
    ProcessEvent* short_e = (ProcessEvent*)shortest_process->events.first;
    assert(short_e->type==CPU);
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
    
    //check if something is running on the current CPU
    if (os->running[cpu]) {
        ProcessEvent* running_e = (ProcessEvent*) os->running[cpu]->events.first;
        //if the process running is shorter than the shortest process in ready, there is no need to switch
        if (running_e->duration < short_duration) return;
        
        printf("\t\tPreempting the current running process: [pid=%d duration=%d]", os->running[cpu]->pid, running_e->duration);
        List_pushFront(&os->ready, (ListItem*) os->running[cpu]);
    }
    
    List_detach(&os->ready, (ListItem*) shortest_process);
    os->running[cpu]=shortest_process;
    printf("\n\t\tTaking the shortest ready process: %d\n", shortest_process->pid);
};

int main(int argc, char** argv) {
    FakeOS_init(&os);
    SchedRRArgs srr_args;
    srr_args.quantum=5;
    os.schedule_args=&srr_args;
    os.schedule_fn=schedSJF;
  
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
    }
}
