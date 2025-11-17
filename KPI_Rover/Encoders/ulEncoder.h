#ifndef ENCODERS_ULENCODER_H_
#define ENCODERS_ULENCODER_H_

#include <stdint.h>

void ulEncoder_Init(void);
void ulEncoder_GetDiffForEthernet(int32_t *diffOutput);

#endif /* ENCODERS_ULENCODER_H_ */
