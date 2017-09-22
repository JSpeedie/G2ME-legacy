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

/** Returns the value of a given struct player's rating (known in the
 * glicko2 formulas as μ (mu))
 *
 * \param *P the struct player whose rating will be returned
 * \return double representing '*P's rating
 */
double getRating(struct player* P) {
	return (P->__rating * 173.7178) + 1500;
}

/** Sets the given struct player's rating to the given value
 *
 * \param *P the struct player whose rating will be changed
 * \param rating the value to set '*P's rating to
 * \return modifies the struct player '*P', returns void
 */
void setRating(struct player* P, double rating) {
	P->__rating = (rating - 1500) / 173.7178;
}

/** Returns the value of a given struct player's rating deviation (known
 * in the glicko2 formulas as Φ (phi))
 *
 * \param *P the struct player whose RD will be returned
 * \return double representing '*P's rating deviation
 */
double getRd(struct player* P) {
	return P->__rd * 173.7178;
}

/** Sets the given struct player's rating deviation to the given value
 *
 * \param *P the struct player whose RD will be changed
 * \param rd the value to set '*P's rating deviation to
 * \return modifies the struct player '*P', returns void
 */
void setRd(struct player* P, double rd) {
	P->__rd = rd / 173.7178;
}

/** Calculates and updates a struct player's rating deviation for the
 * beginning of a rating period.
 *
 * \param *P a struct player representing the player whose RD is to be changed
 * \return modifies struct player '*P', returns void
 */
void _preRatingRD(struct player* P) {
	P->__rd = sqrt(pow(P->__rd, 2) + pow(P->vol, 2));
}

/** Updates a sturct player's rating assuming that player undergoes
 * 'rating_list_size' games with the given outcomes and given opponent ratings
 * and RDs.
 *
 * \param *P a struct player representing the player whose rating is to be changed
 * \param *rating_list an array of doubles containing (in order) all the ratings of
 *     the opponenets '*P' played
 * \param rating_list_size the size of the '*rating_list' array
 * \param *RD_list an array od doubles containing (in order) the RDs of the opponents
 *     '*P' played
 * \param *outcome_list an array of doubles containing (in order) the outcomes
 *     of the games '*P' played. 1 for '*P' winning, 0.5 for a draw and 0
 *     for a '*P' loss
 * \return modifies the given struct player '*P', returns void
 */
void update_player(struct player *P,
	double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list) {

	double adjusted_ratings[rating_list_size];
	double adjusted_RD[rating_list_size];

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
}

/** Calculates the new volatility of a player
 *
 * \param *P the struct player who will have their volatility recalculated
 * \param *rating_list an array of doubles representing the ratings of the
 *     opponents '*P' played
 * \param rating_list_size int representing the size of the '*rating_list'
 *     array
 * \param *RD_list an array of doubles representing the rating deviations
 *     of the opponents '*P' played
 * \param *outcome_list an array of doubles containing (in order) the outcomes
 *     of the games '*P' played. 1 for '*P' winning, 0.5 for a draw and 0
 *     for a '*P' loss
 * \param // TODO fill out this param
 * \return a double representing '*P's new volatility
 */
double _newVol(struct player* P, double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list, double v) {

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

/** Calculates the delta value which represents the "estimated improvement in
 * rating by comparing the pre-period rating to the performance rating based
 * only on game outcomes"
 *
 * \param '*P' the struct player the function will calculate the delta of
 * \param '*rating_list' an array of doubles representing the ratings of the
 *     opponents '*P' played
 * \param 'rating_list_size' int representing the size of the '*rating_list'
 *     array
 * \param '*RD_list' an array of doubles representing the rating deviations
 *     of the opponents '*P' played
 * \param '*outcome_list' an array of doubles containing (in order) the outcomes
 *     of the games '*P' played. 1 for '*P' winning, 0.5 for a draw and 0
 *     for a '*P' loss
 * \param 'v' a double representing the result of a '_v' function call
 * \return a double containing the estimated improvement in '*P's rating
 */
double _delta(struct player* P, double *rating_list, int rating_list_size,
	double *RD_list, double *outcome_list, double v) {

	double tempSum = 0;

	for (int i = 0; i < rating_list_size; i++) {
		tempSum += _g(RD_list[i]) * (outcome_list[i] - _E(P, rating_list[i], RD_list[i]));
	}

	return v * tempSum;
}

/** Calculates the "estimated variance of a the team's/player's rating
 * based only on game outcomes"
 *
 * \param '*P' the struct player who will have their estimated variance
 *     calculated
 * \param '*rating_list' an array of doubles representing the ratings of the
 *     opponents '*P' played
 * \param 'rating_list_size' int representing the size of the '*rating_list'
 *     array
 * \param '*RD_list' an array of doubles representing the rating deviations
 *     of the opponents '*P' played
 * \return a double representing the estimated variance of '*P' based on
 *     game outcomes
 */
double _v(struct player* P, double *rating_list, int rating_list_size, double *RD_list) {

	double tempSum = 0;

	for (int i = 0; i < rating_list_size; i++) {
		double tempE = _E(P, rating_list[i], RD_list[i]);
		tempSum += pow(_g(RD_list[i]), 2) * tempE * (1 - tempE);
	}

	return 1 / tempSum;
}

/** The Glicko2 E function which is a helper function for '_v'
 *
 * \param '*P' the struct player who will have their estimated variance
 *     calculated
 * \param 'p2rating' a double representing the rating of '*P's opponent
 * \param 'p2RD' a double representing the rating deviation of '*P's opponent
 * \return //TODO write this return
 */
double _E(struct player* P, double p2rating, double p2RD) {
	return 1 / (1 + exp(-1 * _g(p2RD) * (P->__rating - p2rating)));
}

/** The Glicko2 g function which is a helper function for '_E'
 *
 * \param 'RD' a double representing a player's rating deviation
 * \return //TODO write this return
 */
double _g(double RD) {
	return 1.00 / sqrt(1 + 3 * pow(RD, 2) / pow(M_PI, 2));
}

/* Applies Step 6 of the algorithm. Use this for players who did not compete
 * in the rating period.
 *
 * \param '*P' the struct player who did not compete
 * \return void
*/
void did_not_compete(struct player* P) {
	_preRatingRD(P);
}
