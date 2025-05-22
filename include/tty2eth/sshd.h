#pragma once

#include "memfs.h"
#include <stddef.h>
#include <stdbool.h>

#define REDIRECT_BUFFER_SIZE 1024

void initSSHD();
bool listenSSHD();

void sshdRedirectOutput(char* buffer, size_t size);
void sshdConfigUpdate(FileDef* file);