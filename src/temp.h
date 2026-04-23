#ifndef TEMP_H
#define TEMP_H

#include <stdbool.h>

extern float temp_c;

void temp_init(void);
bool temp_task(void);

#endif