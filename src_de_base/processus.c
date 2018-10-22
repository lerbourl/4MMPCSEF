#include <cpu.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "segment.h"
#include <stdbool.h>
#include "ecran.h"
#include "horloge.h"
#include "processus.h"

processus process_table[TABLESIZE];
unsigned int pid_actif = 0;
unsigned int processus_dispo = 0;


void idle(void)
{
  for (;;) {
      sti();
      hlt();
      cli();
    }
}

void proc1(void) {
  for (int32_t i = 0; i < 10; i++) {
    printf("[%s] pid = %i sec = %u \n creation processus %d\n", mon_nom(), mon_pid(), get_secondes(), cree_processus(proc3, "proc3"));
    dors(10);
  }
}

void proc2(void) {
  for (int32_t i = 0; i < 4; i++) {
    printf("[%s] pid = %i sec = %u \n", mon_nom(), mon_pid(), get_secondes());
    dors(3);
  }
}

void proc3(void) {
  for (int32_t i = 0; i < 5; i++) {
    printf("[%s] pid = %i sec = %u \n", mon_nom(), mon_pid(), get_secondes());
    dors(1);
  }
}

void fin_proc(){
  process_table[pid_actif].etat = MORT;
  ordonnance();
}

void dors(int i){
  process_table[pid_actif].etat = ENDORMI;
  process_table[pid_actif].wakeUpTime = get_secondes() + i;
  ordonnance();
}

int32_t cree_processus(void (*code)(void), char *nom){
  processus* p;
  for(p = process_table + 1; p < process_table + TABLESIZE ; p++){
    if (p->etat == MORT){
      strcpy(p->nom, nom);
      p->etat = ACTIVABLE;
      p->expile[PILESIZE - 2] = (uint32_t)code;
      p->expile[PILESIZE - 1] = (uint32_t)fin_proc;
      p->regs[1] = (uint32_t)(p->expile + PILESIZE - 2);
      return p->pid;
    }
  }
  return -1;
}

void init_process(){
  /* init idle */
  process_table[0].pid = 0;
  sprintf(process_table[0].nom, "idle");
  process_table[0].etat = ELU;
  processus_dispo++;

  processus* p;
  int i = 1;
  for(p = process_table + 1; p < process_table + TABLESIZE ; p++){
    p->etat = MORT;
    p->pid = i;
    i++;
  }

  cree_processus(proc1, "proc1");
  cree_processus(proc2, "proc2");
  cree_processus(proc3, "proc3");
}

void ordonnance(void){
  int pid_old = pid_actif, new_actif_not_found = 1;
  while(new_actif_not_found){
    if (pid_actif == TABLESIZE - 1) pid_actif = 0;
    else pid_actif++;
    if (process_table[pid_actif].etat == ENDORMI){
      if (process_table[pid_actif].wakeUpTime <= get_secondes()){
        new_actif_not_found = 0;
      }
    }
    else if (process_table[pid_actif].etat != MORT) new_actif_not_found = 0;
  }
  if (process_table[pid_old].etat != ENDORMI && process_table[pid_old].etat != MORT) process_table[pid_old].etat = ACTIVABLE;
  process_table[pid_actif].etat = ELU;
  ctx_sw(process_table[pid_old].regs, process_table[pid_actif].regs);
}

char* mon_nom(){
  return process_table[pid_actif].nom;
}
int mon_pid(){
  return process_table[pid_actif].pid;
}
