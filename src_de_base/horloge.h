#ifndef __HORLOGE_H__
#define __HORLOGE_H__

#define QUARTZ 0x1234DD
#define CLOCKFREQ 50

/*
 * Séance 2 : Gestion du temps
 */

void topright_print(char* chaine);

void tic_PIT(void);

extern void traitant_IT_32(void);

void init_traitant_IT(int32_t num_IT, void (*traitant)(void));

void masque_IRQ(uint32_t num_IRQ, uint32_t masque);

void set_horloge();

void time_init();

uint32_t get_secondes();

#endif
