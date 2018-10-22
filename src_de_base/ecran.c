#include <cpu.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "segment.h"
#include <stdbool.h>
#include "ecran.h"

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
