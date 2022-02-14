#include <assert.h>
#include <stdio.h>

#include "interface.h"

ServerImports imports;
static ServerCallbacks callbacks;

void set_imports(ServerImports *imps)
{
	imports = *imps;
}

ServerCallbacks *get_callbacks(void)
{
	return &callbacks;
}

void set_callbacks(ServerCallbacks *cbs)
{
	callbacks = *cbs;
}