#include <iostream>
#include <stdio.h>
#include "pandohammer/storage.h"
#include "cello.hpp"
#include "cello_pr.hpp"

l2sp_storage(pagerank_config) pagerank_configure;

int CelloMain(int argc, char *argv[])
{
    printf("graph @ %p\n", pagerank_configure.g());
    return 0;
}
