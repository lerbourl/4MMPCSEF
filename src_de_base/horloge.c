#include <cpu.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "segment.h"
#include <stdbool.h>
#include "ecran.h"
#include "horloge.h"
#include "processus.h"

/*
 * Séance 2 : Gestion du temps
 */

 /*
  * varibles globales de temps
  */
 uint32_t sec = 0;
 uint32_t min = 0;
 uint32_t heu = 0;
 uint32_t compteur_pit = 0;

void topright_print(char* chaine){
    uint32_t lig, col;
    curseur_get(&lig, &col);
    place_curseur(0, 71);
    printf("%s", chaine);
    place_curseur(lig, col);
}

uint32_t get_secondes(){
  return sec;
}

void tic_PIT(void){
    outb(0x20, 0x20);
    compteur_pit ++;
    //char chaine[8];
    if (compteur_pit > 50){
        compteur_pit = 0;
        sec++;
        if (sec > 60){
            sec = 0;
            min++;
            if (min > 60){
                min = 0;
                heu++;
            }
        }
        //sprintf(chaine, "%2d:%2d:%2d", heu, min, sec);
        //topright_print(chaine);
    }
    ordonnance();
}

void init_traitant_IT(int32_t num_IT, void (*traitant)(void)){
    /*uint16_t *adress = (uint16_t*)0x1000;
    adress += 4 * num_IT;
    *adress = KERNEL_CS;
    adress++;
    *adress = (uint16_t)traitant;
    adress++;
    *adress = (uint16_t)(traitant >> 16);
    adress++;
    *adress = 0x8E00;
    */
    int32_t * adr_table =(int32_t *)0x1000;
    adr_table+= num_IT*2;
    int32_t mot1=(((int32_t)KERNEL_CS)<<16) | ((int32_t)((int32_t)traitant %65536));
    int32_t mot2=(int32_t)((int32_t)((int32_t)traitant>>16)<<16) | (int32_t)0x8E00;
    *adr_table=mot1;
    *(adr_table+1)=mot2;
    }


void masque_IRQ(uint32_t num_IRQ, uint32_t masque){
    uint8_t IRQ_byte;
    IRQ_byte = inb(0x21);
    if ((uint8_t)masque != (uint8_t)((IRQ_byte & ( 1 << num_IRQ)) >> num_IRQ)){
        IRQ_byte ^= 1 << num_IRQ;
        outb(IRQ_byte, 0x21);
    }
}

void set_horloge(){
    outb(0x34, 0x43);
    int16_t val = (QUARTZ/CLOCKFREQ);
    outb(val%256, 0x40);
    outb((uint8_t)(val >> 8), 0x40);
}

void time_init(){
    // table des interruptions
    init_traitant_IT(32, traitant_IT_32);
    // démasquage de IR0
    masque_IRQ(0, 0);
    //fréquence de l'horloge
    set_horloge();
    // démasquage des interruptions externes
    //sti(); à placer dans le main !

}
