#include <stdio.h>
#include <stdlib.h>
#include "cmd.h"

Cmd *newCmd(void)   { return (Cmd *) malloc(sizeof(Cmd)); }
