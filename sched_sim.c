#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
    int quantum;
} SchedRRArgs;

void schedRR(FakeOS* os, int cpu, void* args_){
    SchedRRArgs* args=(SchedRRArgs*)args_;

    //for (int cpu=0; cpu < NUM_CPUS; ++cpu) {
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
    //}
};

int main(int argc, char** argv) {
    FakeOS_init(&os);
    SchedRRArgs srr_args;
    srr_args.quantum=5;
    os.schedule_args=&srr_args;
    os.schedule_fn=schedRR;
  
    for (int i=1; i<argc; ++i){
        FakeProcess new_process;
        int num_events=FakeProcess_load(&new_process, argv[i]);
        printf("loading [%s], pid: %d, events:%d",
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
