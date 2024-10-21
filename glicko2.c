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

#include <math.h>

typedef struct player {
	/* The system constant, which constrains the change in
	 * volatility over time. */
	double _tau;
	double __rating;
	double __rd;
	double vol;
}Player;


/** Returns the value of a given struct player's rating (μ) on the Glicko-2
 * scale. This function paired with the 'getRd()' function constitute Step 2 of
 * the Glicko-2 Algorithm.
 *
 * \param '*P' the struct player whose rating will be returned.
 * \return double representing '*P's rating.
 */
double getRating(struct player* P) {
	return (P->__rating * 173.7178) + 1500;
}


/** Sets the given struct player's rating (*r*) on the original scale based
 * on the given Glicko-2 scale value. This function paired with the 'setRD()'
 * function constitute Step 8 of the Glicko-2 Algorithm.
 *
 * \param '*P' the struct player whose rating will be changed.
 * \param 'new_rating' the value to set '*P's rating to.
 * \return modifies the struct player '*P', returns void.
 */
void setRating(struct player* P, double new_rating) {
	P->__rating = (new_rating - 1500) / 173.7178;
}


/** Returns the value of a given struct player's rating deviation (Φ) on the
 * Glicko-2 scale. This function paired with the 'setRating()' function
 * constitute Step 2 of the Glicko-2 Algorithm.
 *
 * \param '*P' the struct player whose RD will be returned.
 * \return double representing '*P's rating deviation.
 */
double getRd(struct player* P) {
	return P->__rd * 173.7178;
}


/** Sets the given struct player's rating deviation (RD) on the original scale
 * based on the given Glicko-2 scale value 'new_rd'. This function paired with
 * the 'setRating()' function constitute Step 8 of the Glicko-2 Algorithm.
 *
 * \param '*P' the struct player whose RD will be changed.
 * \param 'new_rd' the value to set '*P's rating deviation to.
 * \return modifies the struct player '*P', returns void.
 */
void setRd(struct player* P, double new_rd) {
	P->__rd = new_rd / 173.7178;
}


/** The Glicko2 *g* function which is used in Steps 3, 4, 7, and by the *E*
 * function.
 *
 * \param 'phi' a double representing a player's rating deviation (Φ) on the
 *     Glicko-2 scale.
 * \return The exact purpose or meaning of the value returned by this function
 *     is not made clear by the Glicko-2 PDF.
 */
double _g(double phi) {
	return 1.00 / sqrt(1 + 3 * pow(phi, 2) / pow(M_PI, 2));
}


/** The Glicko2 *E* function which is used in Steps 3, 4, and 7.
 *
 * \param '*P' the struct player who will have their estimated variance
 *     calculated.
 * \param 'mu_j' a double representing the Glicko-2 scale rating (μ_j) of
 *     '*P's jth opponent.
 * \param 'phi_j' a double representing the Glicko-2 scale rating deviation
 *     (Φ_j) of '*P's jth opponent.
 * \return The exact purpose or meaning of the value returned by this function
 *     is not made clear by the Glicko-2 PDF.
 */
double _E(struct player* P, double mu_j, double phi_j) {
	return 1 / (1 + exp(-1 * _g(phi_j) * (P->__rating - mu_j)));
}


/** Calculates the "estimated variance of a the team's/player's rating
 * based only on game outcomes". This function constitutes Step 3 of the
 * Glicko-2 algorithm.
 *
 * \param '*P' the struct player who will have their estimated variance
 *     calculated
 * \param '*rating_list' an array of doubles representing the ratings of the
 *     opponents '*P' played
 * \param 'rating_list_size' int representing the size of the '*rating_list'
 *     array
 * \param '*rd_list' an array of doubles representing the rating deviations
 *     of the opponents '*P' played
 * \return a double representing the estimated variance of '*P' based on
 *     game outcomes
 */
double _v(struct player* P, double *rating_list, int rating_list_size, \
	double *rd_list) {

	double tempSum = 0;

	for (int j = 0; j < rating_list_size; j++) {
		double tempE = _E(P, rating_list[j], rd_list[j]);
		tempSum += pow(_g(rd_list[j]), 2) * tempE * (1 - tempE);
	}

	return 1 / tempSum;
}


/** Calculates the delta (∆) quantity which representing the estimated
 * improvement in rating based only on game outcomes. This function constitutes
 * Step 4 of the Glicko-2 algorithm.
 *
 * \param '*P' the struct player the function will calculate the delta of.
 * \param '*rating_list' an array of doubles representing the ratings of the
 *     opponents '*P' played.
 * \param 'rating_list_size' int representing the size of the '*rating_list'
 *     array.
 * \param '*rd_list' an array of doubles representing the rating deviations
 *     of the opponents '*P' played.
 * \param '*outcome_list' an array of doubles containing (in order) the outcomes
 *     of the games '*P' played. 1 for '*P' winning, 0.5 for a draw and 0
 *     for a '*P' loss.
 * \param 'v' a double representing the quantity *v* calculated in Step 3 from
 *     this series of outcomes representing the estimated variance of the
 *     player.
 * \return a double containing the estimated improvement in '*P's rating.
 */
double _delta(struct player* P, double *rating_list, int rating_list_size, \
	double *rd_list, double *outcome_list, double v) {

	double tempSum = 0;

	for (int i = 0; i < rating_list_size; i++) {
		tempSum += _g(rd_list[i]) \
			* (outcome_list[i] - _E(P, rating_list[i], rd_list[i]));
	}

	return v * tempSum;
}


/** Calculates the new volatility (σ') of a player. This function constitutes
 * Step 5 of the Glicko-2 algorithm.
 *
 * \param '*P' the struct player who will have their volatility recalculated.
 * \param '*rating_list' an array of doubles representing the ratings of the
 *     opponents '*P' played.
 * \param 'rating_list_size' int representing the size of the '*rating_list'
 *     array.
 * \param '*rd_list' an array of doubles representing the rating deviations
 *     of the opponents '*P' played.
 * \param '*outcome_list' an array of doubles containing (in order) the outcomes
 *     of the games '*P' played. 1 for '*P' winning, 0.5 for a draw and 0
 *     for a '*P' loss.
 * \param 'v' the quantity *v* calculated in Step 3 from a series of outcomes
 *     representing the estimated variance of the player.
 * \return a double representing '*P's new volatility.
 */
double _newVol(struct player* P, double *rating_list, int rating_list_size, \
	double *rd_list, double *outcome_list, double v) {

	double delta = \
		_delta(P, rating_list, rating_list_size, rd_list, outcome_list, v);

	// Step 5, item 1
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


/** Calculates a provisional rating deviation (Φ*) representing the new
 * pre-rating-period value and updates the struct player's rating deviation to
 * it. This function constitutes Step 6 of the Glicko-2 algorithm.
 *
 * \param '*P' a struct player representing the player whose rating deviation
 *     (Φ) is to be changed.
 * \return modifies struct player '*P', returns void.
 */
void _preRatingRD(struct player* P) {
	P->__rd = sqrt(pow(P->__rd, 2) + pow(P->vol, 2));
}


/* Applies Step 6 of the algorithm. Use this for players who did not compete
 * in the rating period.
 *
 * \param '*P' the struct player who did not compete.
 * \return void
*/
void did_not_compete(struct player* P) {
	_preRatingRD(P);
}


/** Updates a struct player's rating assuming that player undergoes
 * 'rating_list_size' games with the given outcomes (s_1, ... , s_m), opponent
 * ratings (μ_1, ... , μ_m), and opponent rating deviations (Φ_1, ... , Φ_m).
 * This function (loosely) constitutes Step 7 of the Glicko-2 algorithm.
 *
 * \param '*P' a struct player representing the player whose rating is to be
 *     changed.
 * \param '*rating_list' an array of doubles containing (in order) all the
 *     ratings of the opponents '*P' played.
 * \param 'rating_list_size' the size of the '*rating_list' array.
 * \param '*rd_list' an array of doubles containing (in order) the RDs of the
 *     opponents '*P' played.
 * \param '*outcome_list' an array of doubles containing (in order) the
 *     outcomes of the games '*P' played. 1 for '*P' winning, 0.5 for a draw
 *     and 0 for a '*P' loss.
 * \return modifies the given struct player '*P', returns void.
 */
void update_player(struct player *P, double *rating_list, \
	int rating_list_size, double *rd_list, double *outcome_list) {

	double v = _v(P, rating_list, rating_list_size, rd_list);

	P->vol = \
		_newVol(P, rating_list, rating_list_size, rd_list, outcome_list, v);

	_preRatingRD(P);

	P->__rd = 1 / sqrt((1 / pow(P->__rd, 2)) + (1 / v));

	double tempSum = 0;
	for (int i = 0; i < rating_list_size; i++) {
		tempSum += _g(rd_list[i]) \
			* (outcome_list[i] - _E(P, rating_list[i], rd_list[i]));
	}
	P->__rating += pow(P->__rd, 2) * tempSum;
}
