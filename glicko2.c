/*
Copyright (c) 2009 Ryan Kirkman

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct player {
	/* The system constant, which constrains the change in
	 * volatility over time. */
	double _tau;
	double __rating;
	double __rd;
	double vol;
}Player;

/* Function prototypes because this is not python */
double _delta(struct player*, double*, int, double*, double*, double);
double _E(struct player*, double, double);
double _g(double);
double _newVol(struct player*, double*, int, double*, double*, double);
double _v(struct player*, double*, int, double*);

// aka mu
double getRating(struct player* P) {
	return (P->__rating * 173.7178) + 1500;
}

void setRating(struct player* P, double rating) {
	P->__rating = (rating - 1500) / 173.7178;
}

// aka phi
double getRd(struct player* P) {
	return P->__rd * 173.7178;
}

void setRd(struct player* P, double rd) {
	P->__rd = rd / 173.7178;
}


void _preRatingRD(struct player* P) {
	/* Calculates and updates the player's rating deviation for the
	beginning of a rating period.
	preRatingRD() -> None */

	P->__rd = sqrt(pow(P->__rd, 2) + pow(P->vol, 2));
}

void update_player(struct player *P,
	double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list) {
	/* Calculates the new rating and rating deviation of the player.
	 * update_player(list[int], list[int], list[bool]) -> None */

	// TODO don't malloc. Static size pls
	double *adjusted_ratings = malloc(rating_list_size * sizeof(double));
	double *adjusted_RD = malloc(rating_list_size * sizeof(double));

	// Convert the rating and rating deviation values for internal use.
	for (int i = 0; i < rating_list_size; i++) {
		adjusted_ratings[i] = (rating_list[i] - 1500) / 173.7178;
		adjusted_RD[i] = RD_list[i] / 173.7178;
	}

	double v = _v(P, rating_list, rating_list_size, RD_list);
	P->vol = _newVol(P, rating_list, rating_list_size, RD_list, outcome_list, v);
	_preRatingRD(P);

	P->__rd = 1 / sqrt((1 / pow(P->__rd, 2)) + (1 / v));

	double tempSum = 0;
	for (int i = 0; i < rating_list_size; i++) {
		tempSum += _g(RD_list[i]) * (outcome_list[i] - _E(P, rating_list[i], RD_list[i]));
	}
	P->__rating += pow(P->__rd, 2) * tempSum;

	free(adjusted_ratings);
	free(adjusted_RD);
}


double _newVol(struct player* P, double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list, double v) {
	/* Calculating the new volatility as per the Glicko2 system.
	_newVol(list, list, list) -> float */

	double delta = _delta(P, rating_list, rating_list_size, RD_list, outcome_list, v);
	double a = log(pow(P->vol, 2));
	double tau = P->_tau;
	double x0 = a;
	double x1 = 0;

	while (x0 != x1) {
		// New iteration, so x(i) becomes x(i-1)
		x0 = x1;
		double d = pow(P->__rating, 2) + v + exp(x0);

		double h1 = -(x0 - a) / pow(tau, 2) - 0.5 * exp(x0) / d + 0.5 * exp(x0) * pow(delta / d, 2);

		double h2 = -1 / pow(tau, 2) - 0.5 * exp(x0) * (pow(P->__rating, 2) + v) / pow(d, 2) + 0.5 * pow(delta, 2) * exp(x0) * (pow(P->__rating, 2) + v - exp(x0)) / pow(d, 3);

		x1 = x0 - (h1 / h2);
	}

	return exp(x1 / 2);
}

double _delta(struct player* P, double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list, double v) {
	/* The delta function of the Glicko2 system.
	_delta(list, list, list) -> float */

	double tempSum = 0;

	for (int i = 0; i < rating_list_size; i++) {
		tempSum += _g(RD_list[i]) * (outcome_list[i] - _E(P, rating_list[i], RD_list[i]));
	}

	return v * tempSum;
}

double _v(struct player* P, double *rating_list, int rating_list_size, double *RD_list) {
	/* The v function of the Glicko2 system.
	_v(list[int], list[int]) -> float */

	double tempSum = 0;

	for (int i = 0; i < rating_list_size; i++) {
		double tempE = _E(P, rating_list[i], RD_list[i]);
		tempSum += pow(_g(RD_list[i]), 2) * tempE * (1 - tempE);
	}

	return 1 / tempSum;
}

// CORRECT
double _E(struct player* P, double p2rating, double p2RD) {
	/* The Glicko E function.
	_E(int) -> float*/

	return 1 / (1 + exp(-1 * _g(p2RD) * (P->__rating - p2rating)));
}

// CORRECT
double _g(double RD) {
	/* The Glicko2 g(RD) function.
	_g() -> float */

	return 1.00 / sqrt(1 + 3 * pow(RD, 2) / pow(M_PI, 2));
}

// CORRECT (HOW TO USE)
void did_not_compete(struct player* P) {
	/* Applies Step 6 of the algorithm. Use this for
	players who did not compete in the rating period.
	did_not_compete() -> None */
	_preRatingRD(P);

}
