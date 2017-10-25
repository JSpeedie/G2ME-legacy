# Table of Contents

* [Technical Description](#technical-description)
* [Examples](#examples)
* [Purpose](#purpose)
* [What it is](#what-it-is)
* [Why it makes Glicko 2 Easier](#why-it-makes-glicko-2-easier)
* [Usage](#usage)
	* [The 'a' flag](#the-a-flag)
	* [The 'b' flag](#the-b-flag)
	* [The 'b' flag](#the-b-flag-1)
	* [The 'g' flag](#the-g-flag)
	* [The 'h' flag](#the-h-flag)
	* [The 'l' flag](#the-l-flag)
	* [The 'p' flag](#the-p-flag)
	* [The 'P' flag](#the-p-flag-1)
	* [The 'r' flag](#the-r-flag)
	* [The 'w' flag](#the-w-flag)
	* [The 'x' flag](#the-x-flag)
* [The Glicko2 System Explained](#the-glicko2-system-explained)
* [The Player File Format](#the-player-file-format)



## Technical Description
C implementation of glicko + the real program that makes handling a glicko system with many people more manageable.


## Examples
\[picture of pr here\]


## Purpose

This program was made out of necessity to make an accurate PR (Power Ranking) for the UTSC Smash Club.



## What it is

If you've ever played dota2, csgo, or followed chess, glicko2 is very similar
to MMR from dota, or ranks from cs go and it is a literal improvement on an
improvement (glicko) of elo from chess.



## Why it makes Glicko 2 Easier

The issue with the current implementations of glicko provided is that they are
just some classes and functions. This code saves player data in binary files
(to reduce storage costs since they store every change in rating), takes bracket
files as inputs and calculates the new glicko ratings for all the players.

*It makes it very easy to simulate a tournament for glicko ratings*



## Usage

### The 'a' flag

`G2ME -a Julian`

Takes a file path, prompts user for space-delimited entry information and then
appends it to the given file. If you use G2ME as intended, you should never
have to use this. Used almost exclusively for debugging and last minute fixes.

### The 'b' flag

`G2ME -b test.br`

This flag takes one input, a bracket file. You aren't restricted by the
extension, but the program expects every line to be of the format

`[player1Name] [player2Name] [player1GameCount] [player2Gamecount] [day] [month] [year]`

It expects for `player1Name` and `player2Name` to be a valid file paths
which it will later read to find the players latest glicko data.
G2ME will either attempt to read the file `player1Name` or upon finding
it does not exist, initialize it to the default values (which set the
player at 1500 350 0.06 (Note that this entry will never be in any player
file as they only store changes)).

An example file

```
Bilal Julian 0 3 1 1 2017
Jon Jonah 3 1 1 1 2017
Steven Andrew 3 2 1 1 2017
Isaiah Santos 3 0 1 1 2017
Ron Julian 4 3 1 1 2017
```

### The 'B' flag

`G2ME -B season.sea`

This flag takes one input, a bracket list file. You aren't restricted by the
extension, but the program expects every line to be of the format

`[file_path_to_a_bracket_file]`

It expects each line to be a valid file path for which it will run
the bracket, updating all the player data. Essentially, for each line
*l* in the argument to `-B` as `-b *l*`.

An example file

```
TSE1.br
TSE2.br
TT1.br
TT2.br
TSE3.br
TT3.br
```

### The 'g' flag

`G2ME -g -b test.br`

The `g` flag tells G2ME to calculate the glicko ratings on a per-game basis.

If not set, the default is to calculate on a by-set basis. This means if the
set count is 3-2, 7-0 or 2-1, glicko only sees this as 1 win for the first
player. The reasoning behind this is that some people sand bag so they can
lose some games because they were messing around, but they usually never let
it cost them the set. Because of this, I believe set count is a good measure,
but it has its downsides. For instance, it takes more than a few games to get enough data to
accurately rate a player and there's no disparity between a player who always
goes 3-2 against someone and a player who always goes 3-0 against the same
someone.

It is up to the user of G2ME to decide which they want to use but I advise
using the g flag only when you have few tournaments with lots of large sets
(best of 5s or 7s).

### The 'h' flag

`G2ME -h Julian`

Stands for Output-File-in-**H**uman-Readable-Form. Often used to output a file
and use various shell commands to parse data for things such as
"Record Against *x-player*" or "Sets Played".

### The 'l' flag

`G2ME -l Julian`

Stands for Output-**L**ast-Line. Not really useful except for debugging. Same
as `-h` except it's only the last line.

### The 'p' flag

`G2ME -p pr`

This flag makes G2ME output a file containing all the players listed in `pr`
with their glicko ratings, RDs, and volatility. This is generally used after
several tournaments to create a finalized ranking. The file format of the file
input (`pr` in this case) has each and every line to be a valid file path to
a player file created by G2ME.

An example input file:

```
Bilal
Julian
Steven
Jon
Ron
Isaiah
Santos
Andrew
```

### The 'P' flag

`G2ME -P pr -b bracket.br`

The P flag is to be used before the `-b` flag. It makes the system adjust
player's data if they were not present in the given bracket. It takes the same
format as the pr file as it is a player list file. For example:

```
Julian
Isaiah
James
```

### The 'r' flag

`G2ME -r Julian`

The r flag takes a player file, prompts the user for a new name and changes
the player file's Player 1 data to have the new name.

### The 'w' flag

`G2ME -w 0.5 -b bracket.br`

This flag requires a following `-b` flag call as it affects how the bracket
will affect player's Glicko2 data. This flag multiplies the change in a
player's Glicko2 data after a set/game by the given value.

### The 'x' flag

`G2ME -x Julian`

This flag will remove the last entry in the given player file.



## The Glicko2 System Explained

In chess they use only one number to represent skill. Your elo.
There are several possible issues with this. The same rating from 2 players
playing for vastly different amounts of time have no tangible difference.
Glicko fixes this by adding `RD` aka Rating Deviation (literally the
Standard Deviation of that players Rating). This allows future calculations
to factor in the certainty of someones rating. The more sure the system is of
the loser's rating, (usually) the larger the change in the winner's rating.
Glicko2 also uses one other number. Volatility. This number usually defaults
to 0.06 (although it depends on the system and who's managing it) and the
higher it is, the more erratic that players results have been. This number
is also used in calculations to reduce the effect on a player who was upset
by a player with erratic placings and inversely, to reduce the increase in
rating for the erratic player who just upset someone.

Long story short, the rating is still the most important number but the manager
can use RD (and/or volatility) to determine if they have enough concrete data
on a player to include them in their pr, for example. Other uses include just
a volatility pr/consistency pr.

## The Player File Format

In case someone wants to reverse-engineer the player files or fork this and
write the player files in a different format, currently it has the first
few bytes as:
1. `sizeof(char)` the length of the player-1's name
2. `len_p1_name` bytes are the characters in the first players name.

After that it takes the repeated form of:
`[len_p2_name][p2_name][p1_rating_after][p1_RD_after][p1_vol_after][p1_game_count][p2_game_count][day][month][year]`
In terms of bytes,

1. `sizeof(char)` the length of the player-2's name
(these are included because there are no new lines, so in order
to find the next entry, we must know the full size in bytes of the current
entry)
2. The same applies for player 2
3. `sizeof(double)` bytes representing player-1's rating
4. `sizeof(double)` bytes representing the player-1's RD
4. `sizeof(double)` bytes representing the player-1's volatility
5. `sizeof(char)` the player-1's game count
6. `sizeof(char)` the player-2's game count
7. `sizeof(char)` the day
8. `sizeof(char)` the month
9. `sizeof(short)` bytes are the year.
This design allows for names containing spaces (although the bracket file
format does not), but more importantly, high precision doubles with minimal file
sizes. Also it's really cool :)



## TODO

* Write examples for README
* Clean up glicko2.c to meet code conventions (line length, doc string,
no TODOs)
* Remove TODOs in G2ME.c, make sure invalid input/error checking is sound
