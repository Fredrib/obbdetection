#include	"main.h"

/************************************************************************/
/* Name:		compute_convex_hull										*/
/* Description:	Compute the convex hull using qhull						*/
/************************************************************************/
void compute_convex_hull(void)
{  
	int dim;  	              /* dimension of points */
	int numpoints;            /* number of points */
	coordT *points;           /* array of coordinates for each point */ 
	boolT ismalloc;           /* True if qhull should free points in qh_freeqhull() or reallocation */ 
	char flags[]= "qhull Tv"; /* option flags for qhull, see qh_opt.htm */
	FILE *outfile= stdout;    /* output from qh_produce_output()			
	                             use NULL to skip qh_produce_output() */ 
	FILE *errfile= stderr;    /* error messages from qhull code */ 
	int exitcode;             /* 0 if no error from qhull */
	facetT *facet;	          /* set by FORALLfacets */
	int curlong, totlong;	  /* memory remaining after qh_memfreeshort */

   	/* initialize dim, numpoints, points[], ismalloc here */
	exitcode= qh_new_qhull (dim, numpoints, points, ismalloc,
							flags, outfile, errfile);
	if (!exitcode) { /* if no error */ 
		/* 'qh facet_list' contains the convex hull */
		FORALLfacets {
			/* ... your code ... */ 
		}
	}
	qh_freeqhull(!qh_ALL);  
	qh_memfreeshort (&curlong, &totlong);
	if (curlong || totlong)
		fprintf (errfile, "qhull internal warning (main): did not free %d bytes of long memory (%d pieces)\n", 
		             totlong, curlong);
};