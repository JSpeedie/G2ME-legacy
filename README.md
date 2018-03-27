# Table of Contents
<details><summary>Click to Expand</summary><p>

* [Technical Description](#technical-description)
* [Examples](#examples)
* [Purpose](#purpose)
* [What it is](#what-it-is)
* [Why it makes Glicko 2 Easier](#why-it-makes-glicko-2-easier)
* [Example Walkthrough](#example-walkthrough)
* [Usage](#usage)
	* [The '0' flag](#the-0-flag)
	* [The 'a' flag](#the-a-flag)
	* [The 'A' flag](#the-A-flag-1)
	* [The 'b' flag](#the-b-flag)
	* [The 'B' flag](#the-b-flag-1)
	* [The 'c' flag](#the-c-flag)
	* [The 'C' flag](#the-c-flag-1)
	* [The 'd' flag](#the-d-flag)
	* [The 'g' flag](#the-g-flag)
	* [The 'h' flag](#the-h-flag)
	* [The 'k' flag](#the-k-flag)
	* [The 'l' flag](#the-l-flag)
	* [The 'm' flag](#the-m-flag)
	* [The 'M' flag](#the-m-flag-1)
	* [The 'n' flag](#the-n-flag)
	* [The 'N' flag](#the-n-flag-1)
	* [The 'o' flag](#the-o-flag)
	* [The 'p' flag](#the-p-flag)
	* [The 'P' flag](#the-p-flag-1)
	* [The 'r' flag](#the-r-flag)
	* [The 'R' flag](#the-r-flag-1)
	* [The 'w' flag](#the-w-flag)
	* [The 'x' flag](#the-x-flag)
* [The Glicko2 System Explained](#the-glicko2-system-explained)
* [The Player File Format](#the-player-file-format)
</p></details>



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

This is a complete implementation of glicko. This code saves player data
in binary files (to reduce storage costs since they store every change in
rating), takes bracket files (or even season files) as inputs and
calculates the new glicko ratings for all the players. It also has features
for outputting this data in meaningful ways such as a list of players from
highest to lowest rating, being able to view a players records against
all other players in the system or even a full table of record/matchup data.

*G2ME makes it easy to track players using Glicko 2 and easy make use
of the data stored by the program*



## Example Walkthrough (And Installation)

<details><summary>**Installation**</summary><p>
To get up and running, launch a terminal and run the following commands

```
$ git clone https://www.github.com/JSpeedie/G2ME G2MEGit
$ cd G2MEGit
$ sudo make all
```

You are now ready to follow the rest of the guide! Note that you should
always cd into the directory containing this project before running
any commands. You can choose not to, but you may have to specify the player
directory for every `G2ME` command you run by using `-d` (which is a hassle).
</p></details>


After installation, the first step is to create the player directory.

```
mkdir .players
```

Second, you need to create a bracket file. For instance, a single elimination
tournament of 4 players could look like this:

```
TheBestPlayer ABadPlayer 3 0 1 1 2018
AGoodPlayer AnOkayPlayer 3 1 1 1 2018
TheBestPlayer AGoodPlayer 3 2 1 1 2018
```

For this example, let's call this file `ExampleBracket`. Depending on how
you want the data calculated, you have several ways of continuing. For the
example, let's assume you want RD adjustments for absence.

```
$ G2ME -b ExampleBracket
```

The Glicko2 data is now stored in 4 files found in `.players/`

```
$ ls -1 .players
TheBestPlayer
AGoodPlayer
AnOkayPlayer
ABadPlayer
```

From here you can interact with the data as you want. Common operations are:

* Printing player info in a human readable form

```
$ G2ME -h TheBestPlayer
TheBestPlayer  ABadPlayer   1662.3  290.3  0.060000  1-0  1/1/2018  ExampleBracket
TheBestPlayer  AGoodPlayer  1791.9  247.5  0.060000  1-0  1/1/2018  ExampleBracket
```

* Creating a pr of all the players in the system

```
$ G2ME -o temp
$ cat temp
TheBestPlayer  1791.9  247.5  0.05999983
  AGoodPlayer  1564.6  245.8  0.05999914
 AnOkayPlayer  1383.4  286.9  0.05999919
   ABadPlayer  1383.4  286.9  0.05999919
```

* Print matchup data for all the players in the system

```
$ G2ME -M
               ABadPlayer  AGoodPlayer  AnOkayPlayer  TheBestPlayer
   ABadPlayer  -           -            -             0-0-1
  AGoodPlayer  -           -            1-0-0         0-0-1
 AnOkayPlayer  -           0-0-1        -             -
TheBestPlayer  1-0-0       1-0-0        -             -
```



## Usage

### The '0' flag

`G2ME -0 -B 2017FallSeason.sea`

Takes no arguments. Players no longer have their RD adjusted for missing
an event. Default behaviour is to apply Step 6 of the Glicko2 Formula
to each player that misses an event.

### The 'a' flag

`G2ME -a Julian`

Takes a file path, prompts user for space-delimited entry information and then
appends it to the given file. If you use G2ME as intended, you should never
have to use this. Used almost exclusively for debugging and last minute fixes.

### The 'A' flag

`G2ME -A Julian`

Takes a file path, and prints the names of all the events this player attended
that were tracked by the system.

Example output:

```
TSE1
TSE2
TT1
TT2
TSE4
```

### The 'b' flag

`G2ME -b test`

This flag takes one input, a bracket file. You aren't restricted by the
extension, but the program expects every line to be of the format:

`[player1Name] [player2Name] [player1GameCount] [player2Gamecount] [day] [month] [year]`

It expects for `player1Name` and `player2Name` to be a valid file paths
which it will later read to find the players latest glicko data.
G2ME will either attempt to read the file `player1Name` or upon finding
it does not exist, initialize it to the default values (which set the
player at 1500 350 0.06 (Note that this default value entry will never
appear in any player file as they only store changes)).

An example bracket file:

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
the bracket, updating all the player data. For our example, in pseudo code
it looks like:

```
for line $i in season.sea
	G2ME -b $i
```

An example file

```
TSE1
TSE2
TT1
TT2
TSE3
TT3
```

### The 'c' flag

`G2ME -c JohnSmith`

This flag takes one input, a player file path. G2ME will then output
the number of non-RD adjustment entries that player has. The purpose of this
is to try to get a grasp of how much data the system has on the player.
If this flag outputs a higher number for one player than another it does not
necessarily mean that it has more accurate data on the one with the
higher number, however.

### The 'C' flag

Takes no arguments. Outputs a csv-style "spreadsheet" of player matchup data.
Useful for turning the data into a spreadsheet or for use with spreadsheet
software. Like `-M`, it is compatible with the `-m` flag, so you can
limit the output to only include players who have attended a specified number
of events.

The data is read left to right. For instance, in the example below, we find
that Julian has 3 wins, 0 ties and 0 losses to Ash, has beaten Bilal 3 times,
tied 0 times and lost 4 times.

```
$ G2ME -m 8 -C
,Bilal,Ash,Andrew,Julian,
Bilal,-,2-0-0,3-0-0,4-0-3,
Ash,0-0-2,-,1-0-1,0-0-3,
Andrew,0-0-3,1-0-1,-,0-0-3,
Julian,3-0-4,3-0-0,3-0-0,-,
```

which in a spreadsheet takes the form of:

|        | Bilal | Ash   | Andrew | Julian |
|:-------|:-----:|:-----:|:------:|:------:|
| **Bilal**  | -     | 2-0-0 | 3-0-0  | 4-0-3  |
| **Ash**    | 0-0-2 | -     | 1-0-1  | 0-0-3  |
| **Andrew** | 0-0-3 | 1-0-1 | -      | 0-0-3  |
| **Julian** | 3-0-4 | 3-0-0 | 3-0-0  | -      |

### The 'd' flag

`G2ME -d players/here/ -b test`

This flag takes one input, a directory file path.
This is where the program will attempt to access, and store player files in.
Useful for keeping the working directory clean or for working on player files
stored on another storage device. By default, if you don't specify the `-d`
flag, the default player directory file path will be `./.players/`. An example
full player file path would be `/home/me/G2ME/.players/JohnSmith`.

### The 'g' flag

`G2ME -g -b test`

The `g` flag tells G2ME to calculate the glicko ratings on a per-game basis.

If not set, the default is to calculate on a by-set basis. This means if the
set count is 3-2, 7-0 or 2-1, glicko only sees this as 1 win for the first
player. The default was chosen to avoid giving glicko points to wins that
in the scheme of things, would never amount to anything. The best player
in the system not trying and losing one game, but winning the set
probably should not give the other player points, but the choice is left
to the user. There are some downsides with this choice however. For instance,
it takes more than a few games to get enough data to
accurately rate a player and there's no disparity between a player who always
goes 3-2 against someone and a player who always goes 3-0 against the same
person.

It is up to the user of G2ME to decide which they want to use but I advise
using the `-g` flag only if you have few tournaments with lots of large sets
(best of 5s or 7s).

### The 'h' flag

`G2ME -h Julian`

Stands for Output-File-in-**H**uman-Readable-Form. Can be used with shell
commands for data parsing.

Example output (from command above):

```
Julian  Mirza    1698.8  69.3  0.059960  0-1  24/11/2017  TT7
Julian  Jonah    1694.1  69.3  0.059959  0-1  1/12/2017   TT8
Julian  Andrew   1698.9  69.3  0.059957  1-0  1/12/2017   TT8
Julian  Edward   1710.7  68.8  0.059957  1-0  1/12/2017   TT8
Julian  Mirza    1700.3  68.4  0.059955  0-1  1/12/2017   TT8
```

### The 'k' flag

Takes no arguments. By default, `G2ME` deletes every file in `player_dir`
removing the usual step of `$ rm .players/*`, but if you want to run
consecutive brackets/bracket file, you can use this flag to prevent `G2ME`
from deleting all the files.

### The 'l' flag

`G2ME -l Julian`

Stands for Output-**L**ast-Line. Not really useful except for debugging. Same
as `-h` except it's only the last line.

### The 'm' flag

`G2ME -p pr -m 3 -o prForPlayersWhoAttendedAtLeast3Events`

Stands for **m**inimum events attended. Useful for outputting a meaningful pr
that won't have the people who only showed up once or twice. Useless flag
unless used with another flag that makes use of it such as `-o` or `-M`.

### The 'M' flag

`G2ME -m 8 -M`

Stands for **M**atchup table. Can be used on its own and requires no arguments.
Outputs the matchups records of all the players in the system, possibly
filtered with the `-m` flag.

Example output:

```
        Ash    Bilal  Jonah  Julian
Ash     -      0-0-2  -      0-0-3
Bilal   2-0-0  -      0-0-2  4-0-3
Jonah   -      2-0-0  -      4-0-0
Julian  3-0-0  3-0-4  0-0-4  -
```

### The 'n' flag

`G2ME -n -R Julian`

Stands for **n**o-colour. By default, G2ME will colour certain inputs to make
interpretation easier. This flag disables that.

### The 'N' flag

When printing record data, instead of the standard:
"**[wins]**-**[ties]**-**[losses]**", if this flag is used, `G2ME` will print
"**[wins]**-**[losses]**" to accomodate users who participate in an event
that can never have ties.

### The 'o' flag

```
G2ME -p pr -o 2017pr
G2ME -o 2017pr
```

Stands for **o**utput-path. The `-o` flag can be used in conjunction
with the `-p` flag to, for instance, filter out players in the system who
are not eligible to be on the pr. Takes an argument of a file path where a
pr is to be
written. When the 2 flags are used together, `G2ME` outputs a sorted list of
all the players listed in the file specified by `-p` into the file specified
by `-o`. It can be used without the `-p` flag which will make it output a pr
containing all players in `player_dir`.

Example output (aka `2017pr` after running the command above):

```
   Jon  2119.9   95.0  0.05998555
 Jonah  1948.9   81.9  0.05997680
Isaiah  1870.7   81.6  0.05997892
  Josh  1800.0   90.0  0.05998035
 Ralph  1776.5  101.5  0.05998667
  DeZy  1770.8   90.9  0.05998575
Julian  1719.9   69.9  0.05996194
Edward  1695.6   86.7  0.05998215
 Bilal  1669.7   72.6  0.05998048
Jerome  1622.6   87.8  0.05999483
 Kriss  1580.8   83.7  0.05997938
```

### The 'p' flag

`G2ME -p pr`

This flag is used in conjunction with the `-o` flag to output a pr. This flag
takes an input of a player list file where each line is a file path to
a player file created by `G2ME`.

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

`G2ME -P pr -b bracket`

The `-P` flag is to be used before the `-b` flag. It makes the system adjust
player's data if they were not present in the given bracket. It takes the same
format as the pr file as it is a player list file. For example:

```
Julian
Isaiah
James
```

Now when doing the Glicko 2 calculations, if `Julian`, `Isaiah`, or `James`
did not attend a given event, they will receive the absence punishment which
in Glicko 2 terms means a function is applied to their rd that raises it
slightly.

### The 'r' flag

`G2ME -r Julian`

The `-r` flag takes a player file, prompts the user for a new name and changes
the player file's Player 1 data to have the new name.

### The 'R' flag

`G2ME -R Julian`

The `-R` flag takes a player file, and prints the set or game (if you used
the `-g` flag to run the brackets) counts for the given player against
every player they have played that the system knows about. Most useful
for getting stats for commentary or for smack talking :^].

Example output:

```
Julian vs Jon = 0-0-3
Julian vs Theo = 9-0-0
Julian vs Bilal = 1-0-3
Julian vs John = 4-0-0
```

### The 'w' flag

`G2ME -w 0.5 -b bracket`

This flag requires a following `-b` flag call as it affects how the bracket
will affect player's Glicko2 data. This flag multiplies the change in a
player's Glicko2 data after a set/game by the given value. **This flag is
not recommended for use.**

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
3. `sizeof(short)` the number of opponents in this players record
4. repeated strings ending in `\0` representing the opponent names,
in order
4. `sizeof(short)` the number of tournaments in this players record
5. repeated strings ending in `\0` representing the tournament names,
in order

After that it takes the repeated form of:
`[p2_id][p1_rating_after][p1_RD_after][p1_vol_after][p1_game_count][p2_game_count][day][month][year][event_id]`
In terms of bytes:

1. `sizeof(short)` bytes representing the player 2 id
2. `sizeof(double)` bytes representing player-1's rating
3. `sizeof(double)` bytes representing the player-1's RD
3. `sizeof(double)` bytes representing the player-1's volatility
4. `sizeof(char)` the player-1's game count
5. `sizeof(char)` the player-2's game count
6. `sizeof(char)` the day
7. `sizeof(char)` the month
8. `sizeof(short)` bytes are the year.
9. `sizeof(short)` the event id



## TODO

* Write examples for man pages, finish examples for README
* Add option for user to set time period where a player receives
no RD adjustments for missing events (default 1 or 2 weeks?)
* Clean up glicko2.c to meet code conventions (line length, doc string,
no TODOs)
* Remove TODOs in G2ME.c, make sure invalid input/error checking is sound
