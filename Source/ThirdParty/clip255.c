
#include <yangutil/yangtype.h>
// Fonction pour limiter la valeur dans la plage [0, 255]
uint8_t clip255(long val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}
