#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"

int generate_binary(char *filepath, Program *root, BuildTargetType target)
{
    FILE *fp;
	DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    fp = fopen(filepath, "w");
    if (fp == NULL) {
        fprintf(stderr, "file open failed:[%s].\n", filepath);
        exit(1);
    }

    fclose(fp);

	DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return 0;
}
