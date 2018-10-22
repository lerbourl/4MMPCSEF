#ifndef __ECRAN_H__
#define __ECRAN_H__

#define FOND 7
#define COULEUR 12
#define W 80
#define H 25

uint16_t *ptr_mem(uint32_t lig, uint32_t col);

void place_curseur(uint32_t lig, uint32_t col);

void curseur_get(uint32_t* lig, uint32_t* col);

uint8_t octet_format(uint8_t clignote, uint8_t fond, uint8_t texte);

void efface_ecran();

void defilement(void);

void traite_car(char c);

void console_putbytes(char *chaine, int32_t taille);

#endif
