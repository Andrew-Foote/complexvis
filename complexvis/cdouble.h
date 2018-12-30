#include <math.h>

struct cdouble {
	double re;
	double im;
};

struct cdouble cdouble_cons(double re, double im);
struct cdouble cdouble_add(struct cdouble x, struct cdouble y);
struct cdouble cdouble_mul(struct cdouble x, struct cdouble y);
double cdouble_abs(struct cdouble z);
struct cdouble cdouble_exp(struct cdouble z);
char *cdouble_str(struct cdouble z);
