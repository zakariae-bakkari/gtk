#ifndef SCREEN_HOME_H
#define SCREEN_HOME_H

#include "../../widgets/headers/common.h"

/* Home and simulation screens */
Widget screen_accueil_create(void);

/* Navigation functions (defined in main.c) */
void nav_to_accueil(void);
void nav_to_bassin(void);

#endif // SCREEN_HOME_H
