#include "fake_process.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  FakePCB* running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;

  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);       //Inizializza il FakeOS os (tutti i campi a 0 e le liste vuote)

void FakeOS_simStep(FakeOS* os);    //Simula la creazione e scheduling di processi in un SO, avanzando la simulazione un passo alla volta

void FakeOS_destroy(FakeOS* os);    //Vuota. Dovrebbe eliminare le strutture del falso SO (?), ma non è stata scritta
