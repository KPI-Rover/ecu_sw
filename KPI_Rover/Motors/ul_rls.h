#ifndef UL_RLS_H
#define UL_RLS_H

#include <stdint.h>

typedef struct ulRLS_s {
    float theta[2];
    float P[2][2];
    float lambda;

    float last_w;
    float last_u;
    uint8_t initialized;
} ulRLS_t;

void ulRLS_Init(ulRLS_t* rls);
void ulRLS_Update(ulRLS_t* rls, float w_k, float u_k_1);

#endif // UL_RLS_H
