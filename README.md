# Table of Contents
<details><summary>Click to Expand</summary><p>

* [Screenshots](#screenshots)
* [Technical Description](#technical-description)
* [Purpose](#purpose)
* [What it is](#what-it-is)
* [Why it makes Glicko 2 Easier](#why-it-makes-glicko-2-easier)
* [Installation](#installation)
* [Example Walkthroughs](#example-walkthroughs)
	* [FAQ/General Usage Walkthrough](#faqgeneral-usage-walkthrough)
* [Converting Challonge Brackets](#converting-challonge-brackets)
* [Converting Smash.gg Events](#converting-smashgg-events)
* [Usage](#usage)
	* [The '0' flag](#the-0-flag)
	* [The 'A' flag](#the-A-flag-1)
	* [The 'b' flag](#the-b-flag)
	* [The 'B' flag](#the-b-flag-1)
	* [The 'c' flag](#the-c-flag)
	* [The 'C' flag](#the-c-flag-1)
	* [The 'd' flag](#the-d-flag)
	* [The 'e' flag](#the-e-flag)
	* [The 'f' flag](#the-f-flag)
	* [The 'g' flag](#the-g-flag)
	* [The 'h' flag](#the-h-flag)
	* [The 'k' flag](#the-k-flag)
	* [The 'm' flag](#the-m-flag)
	* [The 'M' flag](#the-m-flag-1)
	* [The 'n' flag](#the-n-flag)
	* [The 'N' flag](#the-n-flag-1)
	* [The 'o' flag](#the-o-flag)
	* [The 'R' flag](#the-r-flag-1)
	* [The 'v' flag](#the-v-flag)
	* [The 'w' flag](#the-w-flag)
* [The Glicko2 System Explained](#the-glicko2-system-explained)
* [The Player File Format](#the-player-file-format)
	* [Player File: Starter Data](#player-file-start-data)
	* [Player File: Entries](#player-file-entries)
* [Data Files and Their Formats](#data-files-and-their-formats)
	* [Opponent File Format](#opponent-file-format)
	* [Opponent ID File Format](#opponent-id-file-format)
	* [Tournament File Format](#tournament-file-format)
	* [Tournament ID File Format](#tournament-id-file-format)
	* [Season File Format](#season-file-format)
</p></details>

## Screenshots

![Power Rankings Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MEPowerRankingsPic.png)
![Run Brackets Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MERunBracketsPic.png)
![Player Info Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MEPlayerInfoPic.png)

## Technical Description
C implementation of glicko 2 + the real program that makes handling a glicko
system with many people more manageable.


## Purpose

This program was made out of a necessity to make an accurate power ranking
(list of players from highest to lowest rating) for the UTSC Smash Club.


## What it is

If you've ever played dota2, csgo, or followed chess, glicko2 is very similar
to MMR from dota, ranks from cs go, and is version 2 of an improvement
of elo from chess.



## Why it makes Glicko 2 Easier

Not only is this a complete implementation of glicko 2, (as opposed to just the
glicko2 mathematical functions rewritten in a programming language), but it is
a data management system. This program comes with features outside of glicko2,
providing the user with more data.

This program saves player data in binary files, takes bracket files (or season files, a list of bracket files)
as inputs, and calculates the glicko ratings for all the players. It also
has features for outputting this data in meaningful ways such as a list of
players from highest to lowest rating, being able to view a players records
against all other players in the system, or even a full table of record/matchup
data for all the players in the system.

*G2ME provides a way to track players over periods of time using Glicko 2.*



## Installation

<details><summary>Linux/MacOS (Click to Expand)</summary><p>
To get up and running, launch a terminal and run the following commands

```
$ git clone https://www.github.com/JSpeedie/G2ME G2MEGit
$ cd G2MEGit
$ make
$ sudo make install
```

You are now ready to use `G2ME`! You can skip to the Walkthrough section, if you
would like.

Note that if you plan on using this program from the terminal, you should
always `cd` into the directory containing this project before running any
commands. You can choose not to, but you may have to specify the player
directory for every `G2ME` command you run by using `-d` (which is a hassle).
</p></details>

<details><summary>Windows (Click to Expand)</summary><p>
It's recommended you use the GUI and set the binary path to be one of the 2
exe's provided. If you wish to use it in the shell for windows, it's even
simpler. Just download one of the exe's, change into its directory and you're
off to the races. Just replace the "G2ME" command at the start of all examples
with "G2ME32.exe" or "G2ME64.exe".

You are now ready to use `G2ME`! You can skip to the Walkthrough section, if you
would like.

Note that if you plan on using this program from the terminal, you should
always `cd` into the directory containing this project before running any
commands. You can choose not to, but you may have to specify the player
directory for every `G2ME` command you run by using `-d` (which is a hassle).
</p></details>

## Example Walkthroughs

<details><summary>Starter Walkthrough (Click to Expand)</summary><p>
First you need to create a bracket file. For instance, a single elimination
tournament of 4 players could have a bracket file like this:

```
TheBestPlayer ABadPlayer 3 0 28 3 2018
AGoodPlayer AnOkayPlayer 3 1 28 3 2018
TheBestPlayer AGoodPlayer 3 2 28 3 2018
```

Note the structure: `[p1name] [p2name] [p1score] [p2score] [day] [month] [year]`.  
Let's call this file `ExampleBracket`. Depending on how you want the data
calculated, you have several ways of continuing. For the example, let's assume
you want RD adjustments for absence (standard Glicko2 practice).

```
$ G2ME -b ExampleBracket
```

The Glicko2 data is now stored in 4 files found in `.players/`...

```
$ ls -1 .players/
TheBestPlayer
AGoodPlayer
AnOkayPlayer
ABadPlayer
```

...along with some other data in `.data/`, but you shouldn't manually edit the
data in either directory.

From here, you can interact with the data as you want. Common operations are:

* Printing a player's outcome/set history

```
$ G2ME -h TheBestPlayer
TheBestPlayer  ABadPlayer   1662.3  290.3  0.060000  3-0  28/3/2018  ExampleBracket
TheBestPlayer  AGoodPlayer  1791.9  247.5  0.060000  3-2  28/3/2018  ExampleBracket
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

* Printing a single player's head-to-heads/records

```
$ G2ME -R AGoodPlayer
AGoodPlayer vs AnOkayPlayer = 1-0-0
AGoodPlayer vs TheBestPlayer = 0-0-1
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
</p></details>

<details><summary>Season Walkthrough (Click to Expand)</summary><p>

The previous walkthough is fine if you only ever want to run one small bracket,
but if you want a season of brackets, running `G2ME -kb ...` many times
becomes very tedious. `G2ME` has a solution however. Introducing the `-B` flag!
The "B" may as well stand for **B**read and (dairy-free) **B**utter because
it is the flag you will likely use the most.

Say after ExampleBracket, there were 2 more brackets SecondBracket and
ThirdBracket. While you could run:

```
$ G2ME -b ExampleBracket
$ G2ME -kb SecondBracket
$ G2ME -kb ThirdBracket
```

(Note the `-kb` instead of `-b` after the first `-b` call. `G2ME` by default
deletes all player data before running a bracket. This may seem annoying,
but it saves a lot of `rm .players/*` and since the recommended usage is
using `-B`, it's a non-issue.)

The recommended way to do it is to create a season file, a file in which every
line is a file path to a bracket file. For example a file named
`ExampleSeason.sea` could have the contents:

```
ExampleBracket
SecondBracket
ThirdBracket
```

We could then run the season file as input:

```
$ G2ME -B ExampleSeason.sea
running "ExampleBracket" ...DONE
running "SecondBracket" ...DONE
running "ThirdBracket" ...DONE
```

This becomes very useful when you have seasons with lots of events, which,
let's face it, is the only time you should use Glicko2 since all "objective"
rating systems flounder under lack of data.

</p></details>

### FAQ/General Usage Walkthrough

<details><summary>Click to Expand</summary><p>

If you are wondering how to actually make use of the data the system now
tracks, here's how it's done.

This walkthrough will discuss how to do the following:

#### 1. Get the names of the events a given player has attended
<details><summary>Click to Expand</summary><p>

```
$ G2ME -B TSE1ToTSE6
running "TSE1" ...DONE
running "TSE2" ...DONE
running "TSE3" ...DONE
running "TSE4" ...DONE
running "TSE5" ...DONE
running "TSE6" ...DONE
$ G2ME -A [player_name_here]
TSE1
TSE2
TSE3
TSE4
TSE5
TSE6
```

Pretty self-explanatory.
</p></details>

#### 2. Get the number of events a given player has attended
<details><summary>Click to Expand</summary><p>

```
$ G2ME -B TSE1ToTSE6
running TSE1
running TSE2
running TSE3
running TSE4
running TSE5
running TSE6
$ G2ME -c [player_name_here]
6
```

Once again, pretty self-explanatory.
</p></details>

#### 3. Output a csv-style "spreadsheet" of player matchup data
<details><summary>Click to Expand</summary><p>

```
$ G2ME -b ExampleBracket
$ G2ME -C
,ABadPlayer,AGoodPlayer,AnOkayPlayer,TheBestPlayer,
ABadPlayer,-,-,-,0-0-1
AGoodPlayer,-,-,1-0-0,0-0-1
AnOkayPlayer,-,0-0-1,-,-,
TheBestPlayer,1-0-0,1-0-0,-,-,
```

You can save the output of this command to a file, and open it in Excel (or
your favourite alternative) to make a clean spreadsheet of matchup data.
</p></details>

#### 4. Calculate Glicko2 data using game counts instead of set counts
<details><summary>Click to Expand</summary><p>

```
$ G2ME -g -b ExampleBracket
```

Start any G2ME bracket-running command (`-b`, `-B`)
with `-g` to calculate the Glicko2 numbers using
game counts rather than set counts. This is **not** recommended as
most of the time, it has been found to produce less accurate results
and it is susceptible to players sandbagging.
</p></details>

#### 5. Output a given players full Glicko2 history
<details><summary>Click to Expand</summary><p>

```
$ G2ME -b ExampleBracket
$ G2ME -h TheBestPlayer
TheBestPlayer  ABadPlayer   1662.3  290.3  0.060000  1-0  28/3/2018  ExampleBracket
TheBestPlayer  AGoodPlayer  1791.9  247.5  0.060000  1-0  28/3/2018  ExampleBracket
```

Pretty self-explanatory. The rating numbers represent their rating after the
given outcome. In this example, TheBestPlayer's rating was 1662.3 *after*
they won over ABadPlayer.
</p></details>

#### 6. Create a PR of players who have attended at least *x* events
<details><summary>Click to Expand</summary><p>

```
$ G2ME -B ExampleSeason.sea
running "ExampleBracket" ...DONE
running "SecondBracket" ...DONE
running "ThirdBracket" ...DONE
$ G2ME -m [x_here] -o pr_output
```

Same output a `-o` but only includes players who have attended
at least *x* events.
</p></details>

#### 7. Create a PR of a certain group of players
<details><summary>Click to Expand</summary><p>

```
$ vim [filter_file_here]
$ cat [filter_file_here]
TheBestPlayer
$ G2ME -b ExampleBracket
$ G2ME -f [filter_file_here] -O
TheBestPlayer  1791.9  247.5  0.05999983
```

Same output a `-O`, but only includes players whose name matches
one of the lines in `[filter_file_here]`.
</p></details>

#### 8. Get a given player's matchup data/records against other players
<details><summary>Click to Expand</summary><p>

```
$ G2ME -R TheBestPlayer
TheBestPlayer vs ABadPlayer = 1-0-0
TheBestPlayer vs AGoodPlayer = 1-0-0
```

Once again, pretty self-explanatory. Just outputs a players
record against every player they've played so far.
</p></details>

</p></details>

## Converting Challonge Brackets

<details><summary>Click to Expand</summary><p>

One of the most useful parts of this project is that it works quite nicely with
Challonge, the free bracket site. In this repo there is a simple shell script I
have written that takes the url to a challonge bracket, and converts it
(nearly) into a `G2ME` compliant bracket file. Here's an example:

```
$ sh convchallonge.sh [challonge_url_here] > [output_file_here]
```

Let's take a look at what this would output for our ExampleBracket if we
ran it on Challonge.

```
$ sh convchallonge.sh [challonge_url_here] > [output_file_here]
$ cat [output_file_here]
"TheBestPlayer" "ABadPlayer" 3 0 28 3 2018
"AGoodPlayer" "AnOkayPlayer" 3 1 28 3 2018
"TheBestPlayer" "AGoodPlayer" 3 2 28 3 2018
```

Note that it surrounds every player tag with quotations marks. It
is recommended that you don't use player tags for storing player
data and rather use a player's first name and last name since
they are less prone to change. Here's what a recommended change
could look like when completed:

```
$ vim [output_file_here]
$ cat [output_file_here]
JohnSmith ZachStone 3 0 28 3 2018
AaronAardvark VictoriaSmith 3 1 28 3 2018
JohnSmith AaronAardvark 3 2 28 3 2018
```

Challonge seemingly doesn't always get the date right. If this is
of importance to you, make sure to double check the date when
editing the file.

Once this is all done, you're ready to continue with `G2ME`!
If you'd like to add the data from this bracket to the existing
player data in the system, make sure to use the `-k` flag!
</p></details>


## Converting Smash.gg Events

<details><summary>Click to Expand</summary><p>

This repo contains the file `dlsmashgg.js` which is a Node interactive
command line app that allows the user to convert all the events (brackets,
pools, etc.) at a tournament to `G2ME` compliant bracket file(s). The user
will have to install `node` and `npm`, and ensure they have all the app
dependencies by running `npm install` in the directory of this file. Here's an
example:

```
$ node dlsmashgg.js
Please enter tournament identifier (Ex. "toronto-stock-exchange-20"):
```

The user must now provide the identifying portion of the tournament url. An
example is provided, which is associated with the smashgg url
`https://smash.gg/tournament/toronto-stock-exchange-20`

```
$ node dlsmashgg.js
Please enter tournament identifier (Ex. "toronto-stock-exchange-20"): some-tournament-5
```

Upon typing in the identifier and hitting enter, the user will be shown a list
of all the groups (brackets, pools, etc.) at the event, with their title. In
addition, the option `all` is provided, in case the user wants the data from
every group.

```
$ node dlsmashgg.js
Please enter tournament identifier (Ex. "toronto-stock-exchange-20"): some-tournament-5
    all
    1118894    Melee Singles: Pools
    1121654    Melee Singles: Pools
    1121655    Melee Singles: Pools
    1121656    Melee Singles: Pools
Please enter a group id or "all" (Ex. "1118894", "exit" to exit):
```

For this example, let's just get the data for the last pool in the list, 1121656.


```
$ node dlsmashgg.js
Please enter tournament identifier (Ex. "toronto-stock-exchange-20"): some-tournament-5
    all
    1118894    Melee Singles: Pools
    1121654    Melee Singles: Pools
    1121655    Melee Singles: Pools
    1121656    Melee Singles: Pools
Please enter a group id or "all" (Ex. "1118894", "exit" to exit): 1121656


"name1" "name2" 2 0 2 11 2019
"name3" "name4" 2 1 2 11 2019
"name5" "name6" 2 0 2 11 2019
"name7" "name2" 2 0 2 11 2019
"name1" "name6" 2 0 2 11 2019
"name3" "name5" 2 0 2 11 2019
"name7" "name4" 2 0 2 11 2019
"name2" "name6" 0 2 2 11 2019
"name1" "name3" 2 0 2 11 2019
"name7" "name6" 2 0 2 11 2019
"name4" "name5" 0 2 2 11 2019
"name2" "name3" 0 2 2 11 2019
"name7" "name5" 2 1 2 11 2019
"name6" "name3" 0 2 2 11 2019
"name4" "name1" 1 2 2 11 2019
"name7" "name3" 2 0 2 11 2019
"name5" "name1" 0 2 2 11 2019
"name4" "name2" 2 0 2 11 2019
"name7" "name1" 2 1 2 11 2019
"name5" "name2" 2 0 2 11 2019
"name6" "name4" 2 1 2 11 2019

Write output to file (Ex. "bracket.txt", "skip" to continue):
```

**Note:** every player tag or name is surrounded with quotations marks. Quotes
and spaces must be removed for `G2ME`. It is also recommended that you don't
use player tags for storing player data and rather use a player's first name
and last name since they are less prone to change.  This app attempts to
convert player ids to their names, as set on their smash.gg profile, but not
all users will have theirs set, or set correctly, so use with caution. If this
app could not get a participant's first name and last name, it will instead
output their tag.

The user is now prompted to provide a file path, where this data will be written.
If the user decides that they do not want to write the data anywhere, they can
simply type `skip`.

The user will then be prompted once again to enter a group id, or to exit,
allowing the user to keep using the tool to get the data from a select group
of brackets, if they so desire.

Once this is all done, you're ready to continue with `G2ME`!
If you'd like to add the data from this bracket to the existing
player data in the system, make sure to use the `-k` flag!
</p></details>



## Usage

### The '0' flag

`G2ME -0 -B 2017FallSeason.sea`

Takes no arguments. Players no longer have their RD adjusted for missing
an event. Default behaviour is to apply Step 6 of the Glicko2 Formula
to each player that misses an event.

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
Bilal Julian 0 3 14 9 2017
Jon Jonah 3 1 14 9 2017
Steven Andrew 3 2 14 9 2017
Isaiah Santos 3 0 14 9 2017
Ron Julian 4 3 14 9 2017
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
# If not given a -k flag, reset the player directory (reset player data)
for line $i in season.sea
	G2ME -kb $i
```

Possible contents of `season.sea`:

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

### The 'e' flag

`G2ME -e`

Takes no arguments, removes all player files in the directory
'player_dir' which is either $(pwd)/.players/ or whatever it was set
to by a previous `-d` flag.

### The 'f' flag

`G2ME -f filterUTSCStudents -vO`

This flag is used in conjunction with the `-o` flag to filter the output.
This flag takes an input of a player list file where each line is a file
path (or file path from `player_dir`) to a player file created by `G2ME`.

An example input file:

```
Bilal
Julian
Steven
Jon
Isaiah
Andrew
```

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

Stands for Glicko2 **H**istory. Can be used with shell commands for data
parsing.

Example output (from command above):

```
Julian  Mirza   1698.8   69.3  0.059960  0-3  24/11/2017  TT7
Julian  Jonah   1694.1   69.3  0.059959  0-2  1/12/2017   TT8
Julian  Andrew  1698.8   69.3  0.059957  2-1  1/12/2017   TT8
Julian  Edward  1710.7   68.8  0.059957  2-1  1/12/2017   TT8
Julian  Mirza   1700.3   68.3  0.059955  1-3  1/12/2017   TT8
```

### The 'k' flag

Takes no arguments. By default, `G2ME` deletes every file in `player_dir`
removing the usual step of `$ rm .players/*`, but if you want to run
consecutive brackets/bracket file, you can use this flag to prevent `G2ME`
from deleting all the files.

### The 'm' flag

`G2ME -f pr -m 3 -o prForPlayersWhoAttendedAtLeast3Events`

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
G2ME -f pr -o 2017pr
G2ME -o 2017pr
```

Stands for **o**utput-path. The `-o` flag can be used in conjunction
with the `-f` flag to, for instance, filter out players in the system who
are not eligible to be on the pr. Takes an argument of a file path where a
pr is to be
written. When the 2 flags are used together, `G2ME` outputs a sorted list of
all the players listed in the file specified by `-f` into the file specified
by `-o`. It can be used without the `-f` flag which will make it output a pr
containing all players in `player_dir`.

Example output (aka `2017pr` after running the command above):

```
   Jon  2096.9   86.4  0.05998253
 Jonah  1952.7   78.9  0.05997614
Isaiah  1868.1   81.7  0.05997935
  Josh  1855.0   84.5  0.05997642
 Mirza  1769.3   79.0  0.05997854
Julian  1700.3   68.3  0.05995536
 Bilal  1696.7   68.9  0.05997426
Edward  1664.1   83.9  0.05998085
Jerome  1622.1   89.0  0.05999465
 Kriss  1580.9   84.8  0.05997963
  Theo  1579.3   97.0  0.05997789
```

### The 'R' flag

`G2ME -R Julian`

Stands for "get **R**ecords". The `-R` flag takes a player file, and prints
the set (or game, if you used the `-g` flag to run the brackets) counts for the
given player against every player they have played that the system knows about.
Most useful for getting stats for commentary or for smack talking :^].

Example output:

```
Julian vs Jon = 0-0-3
Julian vs Theo = 9-0-0
Julian vs Bilal = 1-0-3
Julian vs John = 4-0-0
```

### The 'v' flag

Takes no arguments. Modifies the output of some option flags (like
`-h`, `-o`, `-O`, `-R`) to contain more information.

### The 'w' flag

`G2ME -w 0.5 -b bracket`

This flag requires a following `-b` flag call as it affects how the bracket
will affect player's Glicko2 data. This flag multiplies the change in a
player's Glicko2 data after a set/game by the given value. **This flag is
not recommended for use.**

## The Glicko2 System Explained

In chess they often use elo to generate a number that represents your skill.
Some have raised concerns with elo as a rating system. For instance, consider 2
players with the same rating, but who have been playing for vastly different
amounts of time. One who has only just played their first set, the other who
has been playing at this club (or what have you) for years. In ELO, They have
no discernible difference. They have only one number representing
each of them and no way of representing novelty. Glicko2 fixes this by
adding `RD` aka Rating Deviation (literally the Standard Deviation of that
players Rating). This allows future calculations to factor in the certainty of
someones rating. The more sure the system is of the loser's rating, (usually)
the larger the change in the winner's rating.  Glicko2 also uses one other
number. Volatility. This number usually defaults to 0.06 (although it depends
on the system and who's managing it) and the higher it is, the more erratic
that player's results have been. This number is also used in calculations to
reduce the effect on a player who was upset (lost a set/game as the favourite) by
a player with erratic placings and inversely, to reduce the increase in rating
for the erratic player who just upset someone.

Long story short, Glicko2 adds 2 other numbers to help represent a player and
facilitate more accurate ratings. To the end user, the rating is still the most
important number, but the manager can use RD (and/or volatility) to determine
if they have enough concrete data on a player to include them in their pr, for
example. Other uses include just a volatility pr/consistency pr.

## The Player File Format

Player files take the general format of:

`[starter data][entry 1][entry 2][entry 3]...`

Below, the 2 main components of the file are explained in more depth.

### Player File: Starter Data

The starter data section breaks down into the following pieces:

`[starter data] = [len_name][name][num_valid_out][num_valid_attended_events][entries]`

Where:

1. `[len_name]` is `sizeof(char)`, and represents the number of characters
   in this player's name.
2. `[name]` `len_name` bytes, representing the player's name.
3. `[num_valid_out]` is `sizeof(unsigned long)`, and represents the number of
   valid (non-RD-adjustment) outcomes (sets or games, depending on previous
   usage of the system) they have experienced.
4. `[num_valid_attended_events]` is `sizeof(unsigned long)`, and represents the number of
   valid (non-RD-adjustment) events they have attended.
4. `[entries]` is an array of unknown length containing player entries. More
   information on player entries in the next section.

### Player File: Entries

After the starter data, the file takes the repeated form of:

`[entry x] = [p2_id][p1_rating_after][p1_RD_after][p1_vol_after][p1_game_count][p2_game_count][day][month][year][event_id][season_id]`

Where:

1. `[p2_id]` is `sizeof(unsigned short)` bytes representing the player 2 id
2. `[p1_rating_after]` is `sizeof(double)` bytes representing player-1's rating
    after playing the set/game.
3. `[p1_RD_after]` is `sizeof(double)` bytes representing the player-1's RD
    after playing the set/game.
3. `[p1_vol_after]` is `sizeof(double)` bytes representing the player-1's
   volatility after playing the set/game.
4. `[p1_game_count]` is `sizeof(unsigned char)` bytes representing the
   player-1's game count
5. `[p2_game_count]` is `sizeof(unsigned char)` bytes representing the
   player-2's game count
6. `[day]` is `sizeof(unsigned char)` bytes representing the day. Note that the
   first bit of this element is used to mark the entry as a competitive entry
   or an RD adjustment entry. It must be processed accordingly.
7. `[month]` is `sizeof(unsigned char)` bytes representing the month.
8. `[year]` is `sizeof(unsigned short)` bytes representing the year.
9. `[event_id]` is `sizeof(unsigned short)` bytes representing the event id
10. `[season_id]` is `sizeof(unsigned short)` bytes representing the season
	this outcome belongs to.

## Data Files and Their Formats

### Opponent File Format

`[num_opps]([opp_name 1 opp_id][opp_name 1])([opp_name 2 opp_id][opp_name 2])...([opp_name n opp_id][opp_name n])`

Where:

1. `[num_opps]` = `n` is `sizeof(unsigned short)`, the number of (opponent id,
   opponent name) pairs in this file. The size of the array, if you will.
2. `[opp_name x opp_id]` is `sizeof(unsigned short) + MAX_NAME_LEN + 1` bytes
   that form a pair of data on a player, compromised of an unsigned short
   representing that opponents global id, followed by `MAX_NAME_LEN + 1`
   characters containing the opponent's name.

**Note:** opponent id and name pairs are ordered alphabetically by opponent
name, allowing for fast conversion (binary search) from opponent name to
opponent id.

### Opponent ID File Format

`[num_opps][opp_name 1][opp_name 2]...[opp_name n]`

Where:

1. `[num_opps]` = `n` is `sizeof(unsigned short)`, the number of
   opponent names in this file. The size of the array, if you will.
2. `[opp_name x]` is `MAX_NAME_LEN + 1` bytes that
   are `MAX_NAME_LEN + 1` characters containing the opponent's name.

**Note:** opponent names are indexed by their global id for fast (linear)
conversion from opponent id to opponent name.

### Tournament File Format

`[num_tournaments]([tournament_name 1 t_id][tournament_name 1])([tournament_name 2 t_id][tournament_name 2])...([tournament_name n t_id][tournament_name n])`

Where:

1. `[num_tournaments]` = `n` is `sizeof(unsigned short)`, the number of (tournament id,
   tournament name) pairs in this file. The size of the array, if you will.
2. `[tournament_name x t_id]` is `sizeof(unsigned short) + MAX_NAME_LEN + 1` bytes
   that form a pair of data on a player, compromised of an unsigned short
   representing that tournaments global id, followed by `MAX_NAME_LEN + 1`
   characters containing the tournament's name.

**Note:** tournament id and name pairs are ordered alphabetically by tournament
name, allowing for fast conversion (binary search) from tournament name to
tournament id.

### Tournament ID File Format

`[num_tournaments][tournament_name 1][tournament_name 2]...[tournament_name n]`

Where:

1. `[num_tournaments]` = `n` is `sizeof(unsigned short)`, the number of
   tournament names in this file. The size of the array, if you will.
2. `[tournament_name x]` is `MAX_NAME_LEN + 1` bytes that
   are `MAX_NAME_LEN + 1` characters containing the tournament's name.

**Note:** tournament names are indexed by their global id for fast (linear)
conversion from tournament id to tournament name.

### Season File Format

`[latest_season_id]`

Where:

1. `[latest_season_id]` is the latest season id used by the system.


## TODO

* Add option for user to set time period where a player receives
no RD adjustments for missing events (default 1 or 2 weeks?)
* Make sure glicko2.c meets code conventions (line length, doc string,
no TODOs)
* Remove TODOs in G2ME.c, make sure invalid input/error checking is sound
