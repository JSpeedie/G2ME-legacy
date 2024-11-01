## Screenshots

<p align="center">
  <img src="https://raw.githubusercontent.com/wiki/JSpeedie/G2ME-legacy/images/G2ME-terminal-demo.gif" width="66%"/>
  <img src="https://raw.githubusercontent.com/wiki/JSpeedie/G2ME-legacy/images/G2ME-GUI.png" width="66%" />
</p>


## Technical Description

This repo contains:
* A C implementation of the Glicko2 rating algorithms.
* code for compiling the `G2ME` executable which makes handling a Glicko system
  with many people more manageable.
* the code for compiling the `G2ME-server` and `G2ME-client` executables which
  allow for remotely accessing the data in the system.
* A Java `.jar` file that functions as a GUI for using the `G2ME` executable.
* Full documentation for all 3 executables available both on the Git
  Repository's [wiki](https://github.com/JSpeedie/G2ME-legacy/wiki) and in the manpages.


## Purpose

This program was made out of the necessity to rank the competitors in the UofT
Smash Club. The official webpage for Glicko2 has links to implementations of
Glicko2 in a handful of languages, but when this project began the choices were
very limited, and provided no form of storage or management for players in the
system.


## Elements of Note

* Network communication
    * This repo contains both a server and client for `G2ME` which allow for
      remote access to data.
* Threading
    * When crunching a bracket, the program forks, with the parent process
      calculating the new Glicko2 ratings for the attendees based on their
      performances. The child process divides the list of players who did not
      attend by the number of cores on the computer and starts one thread per
      core, each adjusting (number-of-absent-players * 1/number-of-cores)
      players RDs.
    * A thread barrier was implemented using semaphores and mutexes as a means
      of preventing the parent process from calculating the new Glicko2 ratings
      for attendees of the next tournament while the child process from the
      previous tournament was still performing the RD adjustments for those who
      did not attend.
* An implementation of a hashtable using the MurmurHash2 hash algorithm
    * When crunching a bracket, all players' who have had their
      Glicko2 data changed so far have their most recent Glicko2 data stored in
      memory in a hashtable that is searchable through hashing their name. This
      is done to prevent accessing player files to get their latest Glicko2
      data as having them in memory (and especially in a quickly-searchable
      hashtable) means much faster access.
* Custom binary file formats
    * This program makes use of 6 binary file formats to store data efficiently.
* Unit testing and integration testing
    * The Criterion unit-testing framework is used to unit-test base-level
      structs and associated functions and my own custom Python testing
      framework is used to run integration tests on the complete `G2ME` binary.


## Installation, Running, Testing

### Linux/MacOS

##### Installation

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

##### Running the program

You are now ready to use `G2ME`! At this point it may be helpful to visit the
[wiki](https://github.com/JSpeedie/G2ME-legacy/wiki) and specifically the 
[Walkthroughs](https://github.com/JSpeedie/G2ME-legacy/wiki/Walkthroughs)
section to get a sense of how to use `G2ME`.

Note that if you plan on using this program from the terminal, you should
always `cd` into the directory containing this project before running any
commands. You can choose not to, but you may have to specify the player
directory for every `G2ME` command you run by using `-d` (which is a hassle).

##### Testing

You can run all the unit tests and integration tests using the following
command:

```bash
make test
```

### Windows

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


## What are the benefits of using G2ME over using another Glicko-2 project?

The main benefit is that `G2ME` stores the Glicko-2 data of players. This means
I can calculate the Glicko-2 data for a collection of players one week, and if
some of the same players also compete the next week, `G2ME` will automatically
use their latest Glicko-2 values when running the new calculations.

`G2ME` also stores additional data on all players. For instance, `G2ME` stores
the entire Glicko-2 history of all players that have entered the system. It
stores their head-to-heads, the tournaments they've attended, and more, and this
can be useful for further evaluating players.


## The Glicko2 System Briefly Explained

In chess they often use Elo to generate a single number that represents the
skill of a competitor. The Elo system is still widely used in the world of
chess, but it has garnered some criticism since it's invention. For instance,
consider 2 players with the same rating, but who have been playing for vastly
different amounts of time. The first player has just finished their first-ever
set, but the second player has been playing at this club for years. Through the
eyes of Elo, there is no difference between these 2 players. In reality, we
might have good reason to be quite confident in the rating of the second
player, who has achieved their rating over dozens and perhaps even hundreds of
sets. When it comes to the first player, we might not be so sure of their
rating after a single set. Perhaps they were very nervous or they got extremely
lucky. Glicko-2 aims to improve on Elo and solve problems like this by adding
more variables that we associate with each player. Glicko-2 not only has a
rating value, but also an "RD" or Rating Deviation (literally the Standard
Deviation of that players Rating) and a Volatility. The former allows future
Glicko-2 calculations to factor in the certainty of someone's rating. The more
sure the system is of the loser of a set's rating, (usually) the larger the
change in the winner's rating. The volatility value, on the other hand, usually
defaults to 0.06 (although it depends on the system and who's managing it) and
a higher value represents that the player's results have been more erratic.
This number is also used in calculations to reduce the effect on players who
suffered surprising wins or losses to erratic players.

In short, Glicko-2 adds 2 other variables to help represent a player's
perceived skill more accurately, and insodoing, Glicko-2 facilitates more
accurate player ratings. To the end user, the rating variable is still the most
important number, but the manager can use RD (and/or volatility) to determine
if they have enough concrete data on a player to include them in their pr, for
example.
