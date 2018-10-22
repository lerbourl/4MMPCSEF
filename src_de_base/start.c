#include <cpu.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "segment.h"
#include <stdbool.h>
#include "ecran.h"
#include "horloge.h"
#include "processus.h"

void kernel_start(void){
    efface_ecran();
    init_process();
    time_init();
    idle();
    // boucle d’attente
    while (1) hlt();
}
