#include <math.h>
#include <Motors/ulRLS.h>

void ulRLS_Init(ulRLS_t* rls)
{
    rls->theta[0] = 0.8f;  // Початкове a
    rls->theta[1] = 0.2f;  // Початкове b

    // Початкова коваріаційна матриця (високе початкове значення для швидкої адаптації)
    rls->P[0][0] = 500.0f;
    rls->P[0][1] = 0.0f;
    rls->P[1][0] = 0.0f;
    rls->P[1][1] = 500.0f;

    rls->lambda = 0.995f;

    rls->last_w = 0.0f;
    rls->last_u = 0.0f;
    rls->initialized = 0;
}

void ulRLS_Update(ulRLS_t* rls, float w_k, float u_k){

    if (!rls->initialized) {
        rls->last_w = w_k;
        rls->last_u = u_k;
        rls->initialized = 1;
        return;
    }

    float phi0 = rls->last_w;
    float phi1 = rls->last_u;

    float y = w_k;

    float Pphi0 = rls->P[0][0]*phi0 + rls->P[0][1]*phi1;
    float Pphi1 = rls->P[1][0]*phi0 + rls->P[1][1]*phi1;

    float denom = rls->lambda + phi0*Pphi0 + phi1*Pphi1;

    if (fabsf(denom) < 1e-6f)
        denom = (denom >= 0.0f) ? 1e-6f : -1e-6f;

    float K0 = Pphi0 / denom;
    float K1 = Pphi1 / denom;

    float a = rls->theta[0];
    float b = rls->theta[1];

    float y_hat = a*phi0 + b*phi1;
    float eps = y - y_hat;

    float new_a = a + K0 * eps;
    float new_b = b + K1 * eps;

    if (new_a < 0.7f) new_a = 0.7f;
    if (new_a > 1.3f) new_a = 1.3f;
    if (new_b < 0.0f) new_b = 0.0f;
    if (new_b > 3.0f) new_b = 3.0f;

    rls->theta[0] = new_a;
    rls->theta[1] = new_b;

    float P00_new = (rls->P[0][0] - K0 * Pphi0) / rls->lambda;
    float P11_new = (rls->P[1][1] - K1 * Pphi1) / rls->lambda;
    float P01_new = (rls->P[0][1] - K0 * Pphi1) / rls->lambda;

    rls->P[0][0] = P00_new;
    rls->P[1][1] = P11_new;

    rls->P[0][1] = P01_new;
    rls->P[1][0] = P01_new;

    rls->last_w = w_k;
    rls->last_u = u_k;
}
