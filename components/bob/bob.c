#include "bob.h"

#include "persistence.h"

void bob_init(void)
{
    persistence_init("system_control");
}
