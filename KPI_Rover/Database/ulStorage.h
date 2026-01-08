#ifndef __ULSTORAGE_H
#define __ULSTORAGE_H

#include <stdbool.h>

bool ulStorage_init(void);
bool ulStorage_load(void);
bool ulStorage_save(void);
bool ulStorage_erase(void);
bool ulStorage_factoryReset(void);

#endif /* __ULSTORAGE_H */
