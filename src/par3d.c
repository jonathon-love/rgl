
/* Avoid conflict with Rinternals.h */
#undef DEBUG

#include <Rdefines.h>
#include <R.h>
#include <Rinternals.h>

#include "api.h"

/* These defines are not in the installed version of R */

#define _  
#define streql(s, t)	(!strcmp((s), (t)))

EXPORT_SYMBOL SEXP par3d(SEXP call, SEXP op, SEXP args, SEXP env);

/* par3d implementation based on R's par implementation
 *
 *  Main functions:
 *	par3d(.)	
 *	Specify(.)	[ par(what = value) ]
 *	Query(.)	[ par(what) ]
 */

static void par_error(char *what)
{
    error(_("invalid value specified for rgl parameter \"%s\""),  what);
}

#ifdef UNUSED
static void lengthCheck(char *what, SEXP v, int n, SEXP call)
{
    if (length(v) != n)
	error(_("parameter \"%s\" has the wrong length"), what);
}
#endif

static void dimCheck(char *what, SEXP v, int r, int c, SEXP call)
{
    SEXP dim = coerceVector(getAttrib(v, R_DimSymbol), INTSXP);
    if (length(dim) != 2 || INTEGER(dim)[0] != r || INTEGER(dim)[1] != c)
    	error(_("parameter \"%s\" has the wrong dimension"), what);
}

#ifdef UNUSED
static void nonnegIntCheck(int x, char *s)
{
    if (x == NA_INTEGER || x < 0)
	par_error(s);
}

static void posIntCheck(int x, char *s)
{
    if (x == NA_INTEGER || x <= 0)
	par_error(s);
}

static void naIntCheck(int x, char *s)
{
    if (x == NA_INTEGER)
	par_error(s);
}

static void posRealCheck(double x, char *s)
{
    if (!R_FINITE(x) || x <= 0)
	par_error(s);
}

static void nonnegRealCheck(double x, char *s)
{
    if (!R_FINITE(x) || x < 0)
	par_error(s);
}

static void naRealCheck(double x, char *s)
{
    if (!R_FINITE(x))
	par_error(s);
}


static void BoundsCheck(double x, double a, double b, char *s)
{
/* Check if   a <= x <= b */
    if (!R_FINITE(x) || (R_FINITE(a) && x < a) || (R_FINITE(b) && x > b))
	par_error(s);
}
#endif

static void Specify(char *what, SEXP value, SEXP call)
{
 
 /* Do NOT forget to update  ../R/par3d.R */
 /* if you  ADD a NEW  par !! 

 */
    SEXP x;
    int success;
    
    success = 0;

    if (streql(what, "userMatrix")) {
	dimCheck(what, value, 4, 4, call);
	x = coerceVector(value, REALSXP);
	
	rgl_setUserMatrix(&success, REAL(x));
    }
     else warning(_("parameter \"%s\" cannot be set"), what);
 
    if (!success) par_error(what);
    
    return;
} 
 
 
 /* Do NOT forget to update  ../R/par3d.R */
 /* if you  ADD a NEW  par !! */
 
/* These defines are repeated here in a C context */

#define mmTRACKBALL 	1
#define mmPOLAR 	2
#define mmSELECTING 	3
#define mmZOOM 		4
#define mmFOV		5
 
static SEXP Query(char *what)
{
    SEXP value, names;
    int i, mode, success;

    success = 0;
    
    if (streql(what, "FOV")) {
	value = allocVector(REALSXP, 1);
	rgl_getFOV(&success, REAL(value));
    }
    else if (streql(what, "modelMatrix")) {
	value = allocMatrix(REALSXP, 4, 4);
	rgl_getModelMatrix(&success, REAL(value));
    }
    else if (streql(what, "mouseMode")) {
    	PROTECT(value = allocVector(STRSXP, 3));
    	for (i=0; i<3; i++) {
    	    rgl_getMouseMode(&success, &i, &mode); 
    	    switch(mode) {
    	    case mmTRACKBALL:
    	    	SET_STRING_ELT(value, i, mkChar("trackball"));
    	    	break;
    	    case mmPOLAR:
    	    	SET_STRING_ELT(value, i, mkChar("polar"));
    	    	break;
    	    case mmSELECTING:
    	    	SET_STRING_ELT(value, i, mkChar("selecting"));
    	    	break;
    	    case mmZOOM:
    	    	SET_STRING_ELT(value, i, mkChar("zoom"));
    	    	break;
    	    case mmFOV:
    	    	SET_STRING_ELT(value, i, mkChar("fov"));
    	    	break;
    	    default:
    	    	SET_STRING_ELT(value, i, mkChar("none"));
    	    };
    	};    
    	PROTECT(names = allocVector(STRSXP, 3));
    	SET_STRING_ELT(names, 0, mkChar("left"));
    	SET_STRING_ELT(names, 1, mkChar("middle"));  
    	SET_STRING_ELT(names, 2, mkChar("right"));
    	UNPROTECT(2);
    	value = namesgets(value, names);
    	success = 1;
    }
    else if (streql(what, "projMatrix")) {
	value = allocMatrix(REALSXP, 4, 4);
	rgl_getProjMatrix(&success, REAL(value));    
    }
    else if (streql(what, "userMatrix")) {
	value = allocMatrix(REALSXP, 4, 4);
	rgl_getUserMatrix(&success, REAL(value));
    }
    else if (streql(what, "viewport")) {
	value = allocVector(REALSXP, 4);
	rgl_getViewport(&success, REAL(value));
    }
    else if (streql(what, "zoom")) {
	value = allocVector(REALSXP, 1);
	rgl_getZoom(&success, REAL(value));
    }
    else
  	value = R_NilValue;
  	
    	
    if (! success) error(_("unknown error getting rgl parameter \"%s\""),  what);

    return value;
}
  
SEXP par3d(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP value;

    int new_spec, nargs;

    new_spec = 0;
    args = CAR(args);
    nargs = length(args);
    if (isNewList(args)) {
	SEXP oldnames, newnames, tag, val;
	int i;
	PROTECT(newnames = allocVector(STRSXP, nargs));
	PROTECT(value = allocVector(VECSXP, nargs));
	oldnames = getAttrib(args, R_NamesSymbol);
	for (i = 0 ; i < nargs ; i++) {
	    if (oldnames != R_NilValue)
		tag = STRING_ELT(oldnames, i);
	    else
		tag = R_NilValue;
	    val = VECTOR_ELT(args, i);
	    if (tag != R_NilValue && CHAR(tag)[0]) {
		new_spec = 1;
		SET_VECTOR_ELT(value, i, Query(CHAR(tag)));
		SET_STRING_ELT(newnames, i, tag);
		Specify(CHAR(tag), val, call);
	    }
	    else if (isString(val) && length(val) > 0) {
		tag = STRING_ELT(val, 0);
		if (tag != R_NilValue && CHAR(tag)[0]) {
		    SET_VECTOR_ELT(value, i, Query(CHAR(tag)));
		    SET_STRING_ELT(newnames, i, tag);
		}
	    }
	    else {
		SET_VECTOR_ELT(value, i, R_NilValue);
		SET_STRING_ELT(newnames, i, R_NilValue);
	    }
	}
	setAttrib(value, R_NamesSymbol, newnames);
	UNPROTECT(2);
    }
    else {
    	error(_("invalid parameter passed to par3d()"));
    	return R_NilValue/* -Wall */;
    }
    return value;
}
 