#ifndef __PROCESSUS_H
#define __PROCESSUS_H

#define NOMSIZE 32
#define REGSIZE 5
#define PILESIZE 512
#define TABLESIZE 4

extern void ctx_sw(uint32_t*, uint32_t*);

enum etat_processus {ENDORMI, ACTIVABLE, ELU, MORT};

/* structure de processus */
typedef struct _processus{
  int32_t pid;
  char nom[NOMSIZE] ;
  enum etat_processus etat;
  uint32_t regs[REGSIZE];
  uint32_t expile[PILESIZE];
  int wakeUpTime;
} processus;

void fin_proc();
void idle(void);
void proc1(void);
void proc2(void);
void proc3(void);
void init_process();
void ordonnance(void);
char* mon_nom();
int mon_pid();
int32_t cree_processus(void (*code)(void), char *nom);
void dors(int i);

#endif
