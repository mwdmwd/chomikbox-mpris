#include <assert.h>
#include <stdio.h>

#include "interface.h"

static mpris_server_import imports[IM_IMPORT_COUNT];

void register_import(ServerImport index, mpris_server_import function)
{
	assert(index >= 0 && index < IM_IMPORT_COUNT);
	imports[index] = function;
}

void call_import(ServerImport index, uint64_t arg)
{
	assert(index >= 0 && index < IM_IMPORT_COUNT);

	mpris_server_import import = imports[index];
	if(import)
		import(arg);
	else
		printf("Call to unregistered import %d\n", index);
}