#pragma once
#include "linked_list.h"

typedef enum {CPU=0, IO=1} ResourceType;

// event of a process, is in a list
typedef struct {
  ListItem list;
  ResourceType type;
  int duration;
} ProcessEvent;

// fake process
typedef struct {
  ListItem list;
  int pid; // assigned by us
  int arrival_time;
  ListHead events;
} FakeProcess;

int FakeProcess_load(FakeProcess* p, const char* filename);     //Loads the FakeProcess p with info taken from the file filename and returns the number of IO and CPU                                                                 events recorded

int FakeProcess_save(const FakeProcess* p, const char* filename);       //Saves the info from the FakeProcess p into a file filename and returns the number of events                                                                         written in the file
