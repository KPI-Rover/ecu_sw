#ifndef __COLORS_H
#define __COLORS_H

#include "config.h"

#if COLOR_ENABLE == 1
	#define COLOR_RESET "\x1b[0m"
	#define COLOR_RED "\x1b[0;31m"
	#define COLOR_GREEN "\x1b[0;32m"
#else
	#define COLOR_RESET ""
	#define COLOR_RED ""
	#define COLOR_GREEN ""
#endif /* COLOR_ENABLE == 1 */


#endif /* __COLORS_H */
