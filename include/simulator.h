#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "bms_telemetry.h"

/* Simulador de modo demo (idêntico ao simulator.py do BMS_UI).
 * Permite testar o display e a navegação sem barramento CAN conectado. */
void sim_tick(void);

#endif // SIMULATOR_H
