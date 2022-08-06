#pragma once

#include "defines.h"
#include "core/vmemory.h"
#include "resources/resource_types.h"

struct resource_loader;

b8 ResourceUnload(struct resource_loader* Self, resource* Resource, memory_tag Tag);

