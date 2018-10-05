#include <cpu.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "segment.h"
#include <stdbool.h>

#define FOND 7
#define COULEUR 12
#define W 80
#define H 25
#define QUARTZ 0x1234DD
#define CLOCKFREQ 50

/*
 * varibles globales de temps
 */
uint32_t sec = 0;
uint32_t min = 0;
uint32_t heu = 0;
uint32_t compteur_pit = 0;

/*
 * Séance 1 : l'écran
 */

uint16_t *ptr_mem(uint32_t lig, uint32_t col){
    return ((uint16_t*)0xB8000) + col + lig * W;
}

void place_curseur(uint32_t lig, uint32_t col){
    uint16_t pos = col + lig * W;
    /*
     * partie basse
     */
    outb(0x0F, 0x3D4);
    outb((uint8_t)pos,0x3D5);
    /*
     * partie haute
     */
    outb(0x0E, 0x3D4);
    outb((uint8_t)(pos >> 8),0x3D5);
}

void curseur_get(uint32_t* lig, uint32_t* col){
    uint32_t pos;
    uint32_t posh;
    /*
     * partie basse
     */
    outb(0x0F, 0x3D4);
    pos = (uint32_t)inb(0x3D5);
    /*
     * partie haute
     */
    outb(0x0E, 0x3D4);
    posh = (uint32_t)inb(0x3D5);
    pos = pos | (posh << 8);
    *col = pos % W;
    *lig = (pos - *col) / W;
}

uint8_t octet_format(uint8_t clignote, uint8_t fond, uint8_t texte){
    clignote <<= 7;
    fond <<= 4;
    return clignote|fond|texte;
}

void ecrit_car(uint32_t lig, uint32_t col, char c,
        uint32_t fond, uint32_t couleur){
    uint8_t* format_add = (uint8_t*)ptr_mem(lig, col);
    uint8_t* ascii_add = format_add ++;
    /*
     * format et caractère à la bonne adresse!
     */
    *ascii_add = c;
    *format_add = octet_format(0, fond, couleur);
}

void efface_ecran(){
    uint32_t i, j;
    for(i = 0 ; i < H ; i++){
        for(j = 0 ; j < W ; j++){
            ecrit_car(i, j, ' ', FOND, COULEUR);
        }
    }
    place_curseur(0, 0);
}

void defilement(void){
    memmove((void*)0xB8000, ptr_mem(1, 0), 2 * W*(H - 1));
    uint32_t i;
    for(i = 0 ; i < W ; i++) ecrit_car(24, i, ' ', FOND, COULEUR);
}

void traite_car(char c){
    uint32_t lig, col;
    curseur_get(&lig, &col);
    if (c > 31 && c < 127 && col != W - 1){
        ecrit_car(lig, col, c, FOND, COULEUR);
        place_curseur(lig, col + 1);
    }
    else if (c == '\n'){
        if (lig == 24){
            defilement();
            place_curseur(24, 0);
        }
        else place_curseur(lig + 1, 0);
    }
    else if (c == '\t'){
        uint32_t  i;
        for(i = 0 ; i < 4 ; i++) traite_car(' ');
    }
    else if (c == '\b'){
        if (col != 0){
            ecrit_car(lig, col - 1, ' ', FOND, COULEUR);
            place_curseur(lig , col - 1);
        }
    }
}

void console_putbytes(char *chaine, int32_t taille){
    uint32_t i;
    for(i = 0 ; i < taille ; i++) traite_car(chaine[i]);
}

/*
 * Séance 2 : Gestion du temps
 */

void topright_print(char* chaine){
    uint32_t lig, col;
    curseur_get(&lig, &col);
    place_curseur(0, 71);
    printf("%s", chaine);
    place_curseur(lig, col);
}

void tic_PIT(void){
    outb(0x20, 0x20);
    compteur_pit ++;
    if (compteur_pit > 50){
        char chaine[8];
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
        sprintf(chaine, "%2d:%2d:%2d", heu, min, sec);
        topright_print(chaine);
    }
}

extern void traitant_IT_32(void);

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

void kernel_start(void){
    // initialisations
    // table des interruptions
    init_traitant_IT(32, traitant_IT_32);

    // démasquage de IR0
    masque_IRQ(0, 0);
    int8_t mask = inb(0x21);
    (void) mask;

    //fréquence de l'horloge
    outb(0x34, 0x43);
    int16_t val = (0x1234DD/50);
    outb(val%256, 0x40);
    outb((uint8_t)(val >> 8), 0x40);

    efface_ecran();
    // démasquage des interruptions externes
    sti();

    // boucle d’attente
    while (1) {
        hlt();
    }
}
