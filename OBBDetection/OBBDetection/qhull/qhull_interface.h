#ifndef qhull_interface_h
#define qhull_interface_h

#include "main.h"

#include <iostream>
#include <conio.h>

#if defined(__cplusplus)
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include "qhull.h"
#include "mem.h"
#include "qset.h"
#include "geom.h"
#include "merge.h"
#include "poly.h"
#include "io.h"
#include "stat.h"
#if defined(__cplusplus)
}
#endif

void compute_convex_hull(void);

#endif

