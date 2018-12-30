#include <stdlib.h>
#include <math.h>
#include <stdio.h>

struct cdouble {
	double re;
	double im;
};

struct cdouble cdouble_cons(double re, double im) {
	struct cdouble z;
	z.re = re;
	z.im = im;
	return z;
}

struct cdouble cdouble_add(struct cdouble x, struct cdouble y) {
	return cdouble_cons(x.re + y.re, x.im + y.im);
}

struct cdouble cdouble_mul(struct cdouble x, struct cdouble y) {
	return cdouble_cons(x.re * y.re - x.im * y.im, x.re * y.im + x.im * y.re);
}

double cdouble_abs(struct cdouble z) {
	return sqrt(z.re * z.re + z.im * z.im);
}

struct cdouble cdouble_exp(struct cdouble z) {
	return cdouble_cons(exp(z.re) * cos(z.im), exp(z.re) * sin(z.im));
}

char *cdouble_str(struct cdouble z) {
	char *str;
	
	str = malloc(64 * sizeof *str);
	if (!str) return str;

	if (!z.im) {
		snprintf(str, 63, "%.2f", z.re);
	} else if (!z.re) {
		if (z.im == 1) {
			snprintf(str, 63, "i");
		} else {
			snprintf(str, 63, "%.2fi", z.im);
		}
	} else if (z.im == -1) {
		snprintf(str, 63, "%.2f - i", z.re);
	} else if (z.im < 0) {
		snprintf(str, 63, "%.2f - %.2fi", z.re, -z.im);
	} else if (z.im == 1) {
		snprintf(str, 63, "%.2f + i", z.re);
	} else {
		snprintf(str, 63, "%.2f + %.2fi", z.re, z.im);
	}
	return str;
}