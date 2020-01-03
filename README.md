# Table of Contents
<details><summary>Click to Expand</summary><p>

* [Screenshots](#screenshots)
* [Technical Description](#technical-description)
* [Purpose](#purpose)
* [What are the advantages of G2ME?](#what-are-the-advantages-of-g2me)
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
	* [The 'O' flag](#the-o-flag-1)
	* [The 'R' flag](#the-r-flag)
	* [The 's' flag](#the-s-flag)
	* [The 'S' flag](#the-s-flag-1)
	* [The 'v' flag](#the-v-flag)
	* [The 'w' flag](#the-w-flag)
* [The Glicko2 System Explained](#the-glicko2-system-explained)
* [The Player File Format](#the-player-file-format)
	* [Player File: Starter Data](#player-file-starter-data)
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

C implementation of Glicko2 + the real program that makes handling a Glicko
system with many people more manageable.

If you've ever played dota2, csgo, or followed chess, Glicko2 is very similar
to MMR from dota, ranks from cs go, and is version 2 of an improvement
of elo from chess.


## Purpose

This program was made out of a necessity to make an accurate ranking
(list of players from highest to lowest rating) for the UTSC Smash Club. The
official webpage for Glicko2 has links to implementations of Glicko2 in a
handful of languages, but the choices were very limited, and provided no form
of storage or management for players in the system.


## What are the advantages of G2ME?

This program takes simple file input, and saves lots of player information.
This is in stark contrast to existing Glicko2 implementations that only provide
functions for calculating one's new rating, after playing someone else. G2ME
not only allows one to calculate the ratings of people without having to touch
code, but it also stores much more information, including players' Glicko2
history, their head-to-heads, the tournaments they've attended, and more.


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

This file is provided in this repo as `examples/example1`.  
Note the structure: `[p1name] [p2name] [p1score] [p2score] [day] [month] [year]`.  
Depending on how you want the data calculated, you have several ways of
continuing. For the example, let's assume you want RD adjustments for absence
(standard Glicko2 practice).

```
$ G2ME -b examples/example1
```

The Glicko2 data is now stored in 4 files found in `.players/`...

```
$ ls -1 .players/
ABadPlayer
AGoodPlayer
AnOkayPlayer
TheBestPlayer
```

...along with some other data in `.data/`, but you shouldn't manually edit the
data in either directory.

From here, you can interact with the data as you want. Common operations are:

* Printing a player's outcome/set history

```
$ G2ME -h TheBestPlayer
TheBestPlayer  ABadPlayer   1662.3  290.3  0.060000  3-0  28/3/2018  example1
TheBestPlayer  AGoodPlayer  1791.9  247.5  0.060000  3-2  28/3/2018  example1
```

* Creating a pr of all the players in the system

```
$ G2ME -O
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

* Print matchup data for all the players in the system (W-T-L stated in terms
of player in row title, ie. TheBestPlayer has 1 win on ABadPlayer)

```
$ G2ME -M
                ABadPlayer   AGoodPlayer   AnOkayPlayer   TheBestPlayer
   ABadPlayer   -            -             -              0-0-1
  AGoodPlayer   -            -             1-0-0          0-0-1
 AnOkayPlayer   -            0-0-1         -              -
TheBestPlayer   1-0-0        1-0-0         -              -
```

</p></details>

<details><summary>Season Walkthrough (Click to Expand)</summary><p>

The previous walkthough is fine if you only ever want to run one bracket,
but if you want a season of brackets, running `G2ME -kb ...` many times
becomes very tedious. `G2ME` has a solution however. Introducing the `-B` flag!
The "B" may as well stand for **B**read and (dairy-free) **B**utter because
it is the flag you will likely use the most.

Say after the previous bracket (`example1`), there were 2 more brackets
`example2`, and `example3`. While you could run...

```
$ G2ME -b examples/example1
$ G2ME -kb examples/example2
$ G2ME -kb examples/example3
```

(Note the `-kb` instead of `-b` after the first `-b` call. `G2ME` by default
deletes all player data before running a bracket. This may seem annoying,
but it saves a lot of `rm .players/*` and since the recommended usage is
using `-B`, it's a non-issue.)

... the recommended way to do it is to create a season file, a file in which
every line is a file path to a bracket file. For example a file named
`ExampleSeason.sea` could have the contents:

```
examples/example1
examples/example2
examples/example3
```

We could then run the season file as input:

```
$ G2ME -B ExampleSeason.sea
running "examples/example1" ...DONE
running "examples/example2" ...DONE
running "examples/example3" ...DONE
```

This becomes very useful when you have seasons with lots of events, which,
let's face it, is the only time you should use Glicko2 since all "objective"
rating systems struggle under a lack of data.

</p></details>

### FAQ/General Usage Walkthrough

<details><summary>Click to Expand</summary><p>

If you are wondering how to actually make use of the data the system now
has, here's how it's done.

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

#### 2. Get the number of outcomes a player has been involved in
<details><summary>Click to Expand</summary><p>

```
$ G2ME -B TSE1ToTSE6
running "TSE1" ...DONE
running "TSE2" ...DONE
running "TSE3" ...DONE
running "TSE4" ...DONE
running "TSE5" ...DONE
running "TSE6" ...DONE
$ G2ME -c [player_name_here]
45
```

It looks like the player played 45 sets (or games, if `-g` was used).
</p></details>

#### 3. Output a csv-style "spreadsheet" of player matchup data
<details><summary>Click to Expand</summary><p>

```
$ G2ME -b examples/example1
$ G2ME -C
,ABadPlayer,AGoodPlayer,AnOkayPlayer,TheBestPlayer,
ABadPlayer,-,-,-,0-0-1,
AGoodPlayer,-,-,1-0-0,0-0-1,
AnOkayPlayer,-,0-0-1,-,-,
TheBestPlayer,1-0-0,1-0-0,-,-,
```

You can save the output of this command to a file, and open it in Excel (or
your favourite alternative) to make a clean spreadsheet of matchup data.
</p></details>

#### 4. Calculate Glicko2 data using game counts instead of set counts
<details><summary>Click to Expand</summary><p>

```
$ G2ME -g -b examples/example1
running "examples/example1" using games ...DONE
```

Preface any G2ME input command (`-b`, or `-B`)
with `-g` to calculate the Glicko2 numbers using
game counts rather than set counts. This is **not** recommended as
most of the time, it has been found to produce less accurate results
and it is susceptible to players sandbagging.
</p></details>

#### 5. Output a given players full Glicko2 history
<details><summary>Click to Expand</summary><p>

```
$ G2ME -b examples/example1
$ G2ME -h TheBestPlayer
TheBestPlayer  ABadPlayer   1662.3  290.3  0.060000  3-0  28/3/2018  example1
TheBestPlayer  AGoodPlayer  1791.9  247.5  0.060000  3-2  28/3/2018  example1
```

Pretty self-explanatory. The rating numbers represent their rating after the
given outcome. In this example, TheBestPlayer's rating was 1662.3 *after*
they won over ABadPlayer.
</p></details>

#### 6. Create a PR of players who have attended at least *x* events
<details><summary>Click to Expand</summary><p>

```
$ G2ME -B ExampleSeason.sea
running "examples/example1" ...DONE
running "examples/example2" ...DONE
running "examples/example3" ...DONE
$ G2ME -m [x_here] -O
```

Same output as `G2ME -O`, but it only includes players who have attended
at least (`>=`) *x* events.
</p></details>

#### 7. Create a PR of a certain group of players
<details><summary>Click to Expand</summary><p>

```
$ vim [filter_file_here]
$ cat [filter_file_here]
TheBestPlayer
$ G2ME -b examples/example1
$ G2ME -f [filter_file_here] -O
TheBestPlayer  1791.9  247.5  0.05999983
```

Same output as `G2ME -O`, but it only includes players whose name matches
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
Challonge, the free bracket site. To convert challonge brackets to input for
`G2ME`, we will use `convchallonge.py`, but before we use it, some manual
configuration must be done.

These days, challonge is not so lenient with people pinging its servers for
this-and-that.  To get the data we need for this program, we must submit a
proper API request to challonge. *This requires having a challonge account*, and
in some cases, it requires using the account that created the tournament.

### Step 1

You only have to do this once. Open up `convchallonge.py` in your favourite
text editor, and change lines 7 and 8 from:

```
user = "[your_challonge_username]"
key = "[your_api_key]"
```

To contain your actual username and actual api key. Please be careful, your api
key allows one to do **anything** your account could do when logged in,
including deleting brackets. *Do not share this file after making these
changes.* After filling in the info, lines 7 and 8 should look something like:

```
user = "UTSCSmash"
key = "Yfdjo82L8jfJYu8E3wb7fUr3WneFIrq5bQBjwla7"
```

### Step 2

With that done, all you have to do now is run the script with the url to the
bracket you want to convert, and the file path where the output will be
written. For example:

```
$ python convchallonge.py [challonge_url_here] [output_file_here]
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

**Note:** Challonge seemingly doesn't always get the date right. If this is
of importance to you, make sure to double check the date when
editing the file.

Once this is all done, you're ready to continue with `G2ME`!
If you'd like to add the data from this bracket *to the existing
player data in the system*, make sure to use the `-k` flag!
</p></details>


## Converting Smash.gg Events

<details><summary>Click to Expand</summary><p>

This repo contains the file `dlsmashgg.js` which is a Node-based interactive
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
all users will have theirs set (or set correctly), so use with caution. If this
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
to each player that misses an event, increasing their RD, altering future
Glicko2 calculations.

### The 'A' flag

`G2ME -A Julian`

Stands for "events **A**ttended". Takes one argument in the form of a player
file, and prints the names of all the events this player attended that were
tracked by the system.

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

This flag takes one argument in the form of a path to a bracket file. You
aren't restricted by the extension, but the program expects every line to be of
the format:

`[player1Name] [player2Name] [player1GameCount] [player2GameCount] [day] [month] [year]`

G2ME will either attempt to read the file `player1Name` in the *player
directory* or, upon finding it does not exist, initialize it to the default
values (which set the player at 1500 350 0.06 (Note that this default value
entry will never appear in any player file as it is unnecessary)).

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

This flag takes one argument in the form of a path to a bracket list file. You
aren't restricted by the extension, but the program expects every line to be of
the format:

`[file_path_to_a_bracket_file]`

It expects each line to be a valid file path for which it will run
the bracket, updating all the player data. For our example, in pseudo code
it looks like:

```
If not given a -k flag:
	reset the player directory   // delete all player data
for line i in season.sea
	G2ME -kb i
```

Example contents of `season.sea`:

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

This flag takes one input in the form of a player file path. G2ME will then
output the number of non-RD adjustment entries that player has. The purpose of
this is to try to get a grasp of how much data the system has on the player.
If this flag outputs a higher number for one player than another it does not
necessarily mean that it has more accurate data on the one with the higher
number, however.

### The 'C' flag

Stands for **C**SV-head-to-heads. This flag takes no arguments and outputs a
csv-style "spreadsheet" of player head-to-head data. It's useful for turning the
data into a spreadsheet or for use with spreadsheet software.

* This flag can be used with the `-m`, and `-f` flags.

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

|            | Bilal | Ash   | Andrew | Julian |
|:-----------|:-----:|:-----:|:------:|:------:|
| **Bilal**  | -     | 2-0-0 | 3-0-0  | 4-0-3  |
| **Ash**    | 0-0-2 | -     | 1-0-1  | 0-0-3  |
| **Andrew** | 0-0-3 | 1-0-1 | -      | 0-0-3  |
| **Julian** | 3-0-4 | 3-0-0 | 3-0-0  | -      |

### The 'd' flag

`G2ME -d players/here/ -b test`

This flag takes one argument in the form of a directory file path.  The
specified directory is where the program will attempt to access, and store
player files.  It's useful for keeping the working directory clean or for
working on player files stored on another storage device. By default, if you
don't specify the `-d` flag, the default player directory file path will be
`./.players/`.

An example full player file path would be `/home/me/G2ME/.players/JohnSmith`.

### The 'e' flag

`G2ME -e`

Takes no arguments, removes all player files in the directory
'player_dir' which is either $(pwd)/.players/ or whatever it was set
to by a previous `-d` flag.

### The 'f' flag

`G2ME -f currentStudents -vO`

Stands for **f**ilter. Takes one argument in the form of a path to a filter
file. This flag has no effect unless it precedes certain output flags.

* This flag can be used with the `-C`, `-h`, `-o`, `-O`, `-M`, or `-R` output
  flags.

This flag is used in conjunction with the select output flags to filter the
output.  This flag takes an input of a player list file where each line is a
name of a player in the current `G2ME` system.

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

```
$ G2ME -g -b test
running "test" using games ...DONE
```

Stands for use-**g**ames. Takes no arguments, but has no effect unless
it precedes an input flag (`-b` or `-B`).

The `-g` flag tells G2ME to calculate the Glicko ratings on a per-game basis.
If not set, the default is to calculate on a by-set basis. This means if the
set count is 3-2, 7-0 or 2-1, Glicko only sees this as 1 win for the first
player. This default was chosen to avoid giving Glicko points for successes that
in the scheme of things, would never amount to anything. For instance, the best
player in the system not trying, and losing one game against a lesser player,
but winning the set probably should not give the lesser player points. The
decision of whether to use games or sets is still left up to the user.

I advise using the `-g` flag only if you have few tournaments or few sets in
general, but each is large (best of 5s, 7s, and so on).

### The 'h' flag

`G2ME -h Julian`

Stands for **H**istory. Takes one argument in the form of a player
file, and prints the outcome history of the player, including their Glicko2
history.

* This flag can be used with the `-m`, or`-f` flags.

* This flag can be used with the `-v` flag, for more information in the output.

Example output (from command above):

```
Julian  Mirza   1698.8   69.3  0.059960  0-3  24/11/2017  TT7
Julian  Jonah   1694.1   69.3  0.059959  0-2  1/12/2017   TT8
Julian  Andrew  1698.8   69.3  0.059957  2-1  1/12/2017   TT8
Julian  Edward  1710.7   68.8  0.059957  2-1  1/12/2017   TT8
Julian  Mirza   1700.3   68.3  0.059955  1-3  1/12/2017   TT8
```

### The 'k' flag

`G2ME -b examples/example1 -k -b examples/example2`

Stands for **k**eep-player-data. Takes no arguments. By default, `G2ME` deletes
every file in `player_dir` removing the usual step of `$ rm .players/*`, but if
you want to run consecutive brackets/seasons, you can use this flag to
prevent `G2ME` from deleting all the files.

### The 'm' flag

`G2ME -m 3 -o prForPlayersWhoAttendedAtLeast3Events`

Stands for **m**inimum events attended. This flag takes a positive integer as
an argument and filters out players from output that have not attended at least
that many events.  This flag has no effect unless it precedes certain output
flags.

* This flag can be used with the `-C`, `-h`, `-o`, `-O`, `-M`, and `-R` output
flags.

This flag is useful for outputting a meaningful ranking that won't have the
people who only showed up once or twice.

### The 'M' flag

`G2ME -M`

Stands for **M**atchup table. This flag takes no arguments, and outputs the
head-to-head records of all the players in the system.

* This flag can be used with the `-m`, and `-f` flags.

Example output:

```
         Ash     Bilal   Jonah   Julian  
   Ash   -       0-0-2   -       0-0-3   
 Bilal   2-0-0   -       0-0-2   4-0-3   
 Jonah   -       2-0-0   -       4-0-0   
Julian   3-0-0   3-0-4   0-0-4   -       
```

### The 'n' flag

`G2ME -n -R Julian`

Stands for **n**o-colour. This flag takes no arguments. By default, `G2ME` will
colour certain outputs to make interpretation easier. This flag disables that.
Mostly useful to allow parsing of `G2ME` output by other programs.

* This flag has an effect on the `-h`, and `-R` output flags.

### The 'N' flag

`G2ME -N -M`

Stands for **N**o-ties. This flag takes no arguments. If this flag is used,
when printing record data, instead of the standard:
"**[wins]**-**[ties]**-**[losses]**", `G2ME` will print
"**[wins]**-**[losses]**" to accomodate users who participate in an event that
can never have ties.

* This flag has an effect on the `-C`, `-M`, and `-R` output flags.

### The 'o' flag

```
G2ME -o 2017pr
```

Stands for **o**utput-path.  Takes an argument of a file path where a ranking
is to be written.  This flag outputs a sorted list of all the players in the
system from highest Glicko2 rating to lowest to the file specified by the flag.

* This flag can be used with the `-m`, or`-f` flags.

* This flag can be used with the `-v` flag, for more information in the output.

Example file contents (aka `2017pr` after running the command above):

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

### The 'O' flag

```
G2ME -O
```

Stands for **O**utput.  Takes no arguments and outputs a sorted list of all the
players in the system from highest Glicko2 rating, to lowest to stdout. The
output of this flag is the exact same as contents of the file written to by
calling `G2ME -o [some_file]`.

* This flag can be used with the `-m`, and `-f` flags.

* This flag can be used with the `-v` flag, for more information in the output.

Example output:

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

* This flag can be used with the `-m`, and `-f` flags.

* This flag can be used with the `-v` flag, for more information in the output.

Example output:

```
Julian vs Jon = 0-0-3
Julian vs Theo = 9-0-0
Julian vs Bilal = 1-0-3
Julian vs John = 4-0-0
```

### The 's' flag

`G2ME -s -b examples/TEST1`

Stands for "**s**ilent". Takes no arguments, *the next* input flag (`-b` or
`-B`) will not print anything to `stdout`. Note it will only affect the first
following input flag. See the `-S` flag for silencing all input flags.

Example:

```
$ G2ME -b examples/TEST1
running "TEST1" ...DONE
$ G2ME -s -b examples/TEST1
$ G2ME -s -b examples/TEST1 -b examples/TEST2
running "TEST2" ...DONE
```

### The 'S' flag

`G2ME -S -b examples/TEST1 -b examples/TEST2`

Stands for "**S**ilent-all". Takes no arguments, **all** following input flags
(`-b` or `-B`) will not print anything to `stdout`.

Example:

```
$ G2ME -b examples/TEST1
running "TEST1" ...DONE
$ G2ME -S -b examples/TEST1
$ G2ME -S -b examples/TEST1 -b examples/TEST2
```

### The 'v' flag

Stands for "**v**erbose mode". Takes no arguments. Modifies the output of some
output flags to contain more information.

* This flag has an effect on the `-h`, `-o`, `-O`, and `-R` output flags.

### The 'w' flag

`G2ME -w 0.5 -b bracket`

Stands for **w**eight. This flag takes one argument in the form of a double (or
float, if you will) representing how much the change in rating RD, and
volatility for every player should be multiplied by, when running a bracket. As
such, this flag requires a following input flag (`-b`, or `-B`) call. The
intention is that one can value different events more or less than others.
**This flag is not recommended for use.**


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

`[starter data] = [len_name][name][num_valid_out][num_valid_attended_events]`

Where:

1. `[len_name]` is `sizeof(char)`, and represents the number of characters
   in this player's name.
2. `[name]` `len_name` bytes, representing the player's name.
3. `[num_valid_out]` is `sizeof(unsigned long)`, and represents the number of
   valid (non-RD-adjustment) outcomes (sets or games, depending on previous
   usage of the system) they have experienced.
4. `[num_valid_attended_events]` is `sizeof(unsigned long)`, and represents the number of
   valid (non-RD-adjustment) events they have attended.

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
2. `([opp_name x opp_id][opp_name x])` is `sizeof(unsigned short) +
   MAX_NAME_LEN + 1` bytes that form a pair of data on a player, compromised of
   an unsigned short representing that opponents global id, followed by
   `MAX_NAME_LEN + 1` characters containing the opponent's name.

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
2. `([tournament_name 1 t_id][tournament_name 1])` is `sizeof(unsigned short) +
   MAX_NAME_LEN + 1` bytes that form a pair of data on a player, compromised of
   an unsigned short representing that tournaments global id, followed by
   `MAX_NAME_LEN + 1` characters containing the tournament's name.

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
* Move barebones glicko2 C implementation to separate repo for those who just
want that part of the project.
* Remove TODOs in G2ME.c, make sure invalid input/error checking is sound
