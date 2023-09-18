#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"


void FakeOS_init(FakeOS* os) {
    for (int cpu=0; cpu < NUM_CPUS; ++cpu) os->running[cpu] = NULL;
    List_init(&os->ready);
    List_init(&os->waiting);
    List_init(&os->processes);
    os->timer=0;
    os->schedule_fn=0;
    os->avg_ArrivalTime = 0;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
    // sanity check
    assert(p->arrival_time==os->timer && "time mismatch in creation");
    // we check that in the list of PCBs there is no
    // pcb having the same pid
    for (int cpu=0; cpu < NUM_CPUS; ++cpu) assert( (!os->running[cpu] || os->running[cpu]->pid!=p->pid) && "pid taken");

    ListItem* aux=os->ready.first;
    while(aux){
        FakePCB* pcb=(FakePCB*)aux;
        assert(pcb->pid!=p->pid && "pid taken");
        aux=aux->next;
    }

    aux=os->waiting.first;
    while(aux){
        FakePCB* pcb=(FakePCB*)aux;
        assert(pcb->pid!=p->pid && "pid taken");
        aux=aux->next;
    }

    // all fine, no such pcb exists
    FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
    new_pcb->list.next=new_pcb->list.prev=0;
    new_pcb->pid=p->pid;
    new_pcb->events=p->events;
    
    new_pcb->burst = 0;
    new_pcb->preempted = 0;

    assert(new_pcb->events.first && "process without events");

    // depending on the type of the first event
    // we put the process either in ready or in waiting
    ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
    switch(e->type){
        case CPU:
            new_pcb->next_burst = 5;
            List_pushBack(&os->ready, (ListItem*) new_pcb);
            break;
        case IO:
            List_pushBack(&os->waiting, (ListItem*) new_pcb);
            break;
        default:
            assert(0 && "illegal resource");
            ;
    }
}




void FakeOS_simStep(FakeOS* os){
    
    printf("\n******************* TIME: %08d *******************\n\n", os->timer);

    //scan process waiting to be started
    //and create all processes starting now
    ListItem* aux=os->processes.first;
    while (aux){
        FakeProcess* proc=(FakeProcess*)aux;
        FakeProcess* new_process=0;
        if (proc->arrival_time==os->timer){
            new_process=proc;
        }
        aux=aux->next;
        if (new_process) {
            printf("\tcreate pid:%d\n", new_process->pid);
            new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
            FakeOS_createProcess(os, new_process);
            free(new_process);
        }
    }

    // scan waiting list, and put in ready all items whose event terminates
    aux=os->waiting.first;
    while(aux) {
        FakePCB* pcb=(FakePCB*)aux;
        aux=aux->next;
        ProcessEvent* e=(ProcessEvent*) pcb->events.first;
        printf("\twaiting pid: %d\n", pcb->pid);
        assert(e->type==IO);
        e->duration--;
        printf("\t\tremaining time:%d\n",e->duration);
        if (e->duration==0){
            printf("\t\tend burst\n");
            List_popFront(&pcb->events);
            free(e);
            List_detach(&os->waiting, (ListItem*)pcb);
            if (! pcb->events.first) {
                // kill process
                printf("\t\tend process\n");
                free(pcb);
            } else {
                //handle next event
                e=(ProcessEvent*) pcb->events.first;
                switch (e->type){
                    case CPU:
                        printf("\t\tmove to ready\n");
                        List_pushBack(&os->ready, (ListItem*) pcb);
                        break;
                    case IO:
                        printf("\t\tmove to waiting\n");
                        List_pushBack(&os->waiting, (ListItem*) pcb);
                        break;
                }
            }
        }
    }

  

    // decrement the duration of running
    // if event over, destroy event
    // and reschedule process
    // if last event, destroy running
    for (int cpu=0; cpu < NUM_CPUS; ++cpu) {
        printf("CPU: %d\n", cpu);
        FakePCB* running = os->running[cpu];
        printf("\trunning pid: %d\n", running?running->pid:-1);
        if (running) {
            ProcessEvent* e=(ProcessEvent*) running->events.first;
            assert(e->type==CPU);
            
            e->duration--;
            running->burst++;
            printf("\t\tremaining time:%d\n",e->duration);
            printf("\t\tpassed time:%d\n",running->burst);
            if (e->duration==0){
                printf("\t\tend burst\n");
                List_popFront(&running->events);
                free(e);
                if (! running->events.first) {
                    printf("\t\tend process\n");
                    running->next_burst = 0;
                    free(running); // kill process
                } else {
                    //printf("\t\tprevious prediction: %.2f\n", running->next_burst);
                    running->next_burst = running->next_burst * (1-ALPHA) + running->burst * ALPHA;
                    //printf("\t\tnext burst: %.2f\n", running->next_burst);
                    
                    e=(ProcessEvent*) running->events.first;
                    switch (e->type){
                        case CPU:
                            printf("\t\tmove to ready\n");
                            List_pushBack(&os->ready, (ListItem*) running);
                            break;
                        case IO:
                            printf("\t\tmove to waiting\n");
                            List_pushBack(&os->waiting, (ListItem*) running);
                            break;
                    }
                }
                if (!running->preempted) running->burst = 0;
                os->running[cpu] = 0;
            }
        }
        
        int available_cpu = 0;
        for (int i=cpu+1; i<NUM_CPUS; ++i){
            if (os->running[cpu] && !os->running[i]){
                printf("\t\t\t|||Another CPU is available. Skipping scheduling!|||\n");
                available_cpu = 1;
                break;
            }
        }
        if(available_cpu) continue;
        
        // call scheduler, if defined
        if (os->schedule_fn){
            //printf("Calling the scheduler:\n");
            (*os->schedule_fn)(os, cpu, os->schedule_args);
        }
        
        // if scheduler not defined and ready queue not empty
        // put the first in ready to run
        if (! os->running[cpu] && os->ready.first) {
            os->running[cpu]=(FakePCB*) List_popFront(&os->ready);
        }
    }
        
    ++os->timer;
}

void FakeOS_destroy(FakeOS* os) {
}
