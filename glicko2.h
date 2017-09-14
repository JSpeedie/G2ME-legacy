typedef struct player {
	/* The system constant, which constrains the change in
	 * volatility over time. */
	double _tau;
	double __rating;
	double __rd;
	double vol;
}Player;

/* Function prototypes because this is not python */
void did_not_compete(struct player*);
double getRating(struct player*);
double getRd(struct player*);
void setRating(struct player*, double);
void setRd(struct player*, double);
void update_player(struct player*, double*, int, double*, double*);
double _delta(struct player*, double*, int, double*, double*, double);
double _E(struct player*, double, double);
double _g(double);
void _preRatingRD(struct player*);
double _newVol(struct player*, double*, int, double*, double*, double);
double _v(struct player*, double*, int, double*);
