## Screenshots

![Power Rankings Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2ME-terminal-demo.gif)  
![Power Rankings Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MEPowerRankingsPic.png)  
![Run Brackets Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MERunBracketsPic.png)  
![Player Info Pic](https://raw.githubusercontent.com/wiki/JSpeedie/G2ME/images/G2MEPlayerInfoPic.png)


## Technical Description

This repo contains:
* A C implementation of the Glicko2 rating algorithms.
* code for compiling the `G2ME` executable which makes handling a Glicko system
  with many people more manageable.
* the code for compiling the `G2ME-server` and `G2ME-client` executables which
  allow for remotely accessing the data in the system.
* A Java `.jar` file that functions as a GUI for using the `G2ME` executable.
* Full documentation for all 3 executables available both here, further down in
  the README, and in manpages.


## Purpose

This program was made out of the necessity to rank the competitors in the UTSC
Smash Club. The official webpage for Glicko2 has links to implementations of
Glicko2 in a handful of languages, but when this project began the choices were
very limited, and provided no form of storage or management for players in the
system.


## Elements of Note

* Threading
    * When crunching a bracket, the program checks how many cores the computer
      has which it could make use of, and then divides the list of players who
      did not attend the event by the number of cores and starts one thread per
      core, each adjusting 1/corenum players RDs.
* An implementation of a hashtable using the MurmurHash2 hash algorithm
    * When crunching a bracket, all players' who have had their
      Glicko2 data changed so far have their most recent Glicko2 data stored in
      memory in a hashtable that is searchable through hashing their name. This
      is done to prevent accessing player files to get their latest Glicko2
      data as having them in memory (and especially in a quickly-searchable
      hashtable) means much faster access.
* Custom binary file formats
    * This program makes use of 6 binary file formats to store data efficiently.


## Installation

<details><summary>Linux/MacOS (Click to Expand)</summary><p>
To get up and running, launch a terminal and run the following commands

```bash
# If you want to push to the repo
git clone git@github.com:JSpeedie/G2ME.git G2MEGit
# If you are just installing
git clone https://www.github.com/JSpeedie/G2ME G2MEGit
cd G2MEGit
make
sudo make install
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


## What are the advantages of G2ME?

This program takes simple file input, and saves lots of player information.
This is in stark contrast to the Glicko2 implementations that existed at the
start of this project, which only provided functions for calculating one's new
rating after playing someone else. `G2ME` not only allows one to calculate the
ratings of people without having to touch code, but it also *stores* the Glicko2
information *plus* all players' Glicko2 history, their head-to-heads, the
tournaments they've attended, and more.


## The Glicko2 System Briefly Explained

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


## TODO

* Add option for user to set time period where a player receives
no RD adjustments for missing events (default 1 or 2 weeks?)
* Make sure glicko2.c meets code conventions (line length, doc string,
no TODOs)
* Move barebones glicko2 C implementation to separate repo for those who just
want that part of the project.
* Remove TODOs in G2ME.c, make sure invalid input/error checking is sound
