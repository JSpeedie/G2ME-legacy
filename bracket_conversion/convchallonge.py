import sys
import requests

# Usage: 'python convchallonge.py https://www.challonge.com/brackethere outputfile'

# grab your api-key here: https://challonge.com/settings/developer
user = "[your_challonge_username]"
key = "[your_api_key]"
base = "https://" + user + ":" + key + "@api.challonge.com/v1/tournaments/"

if ((len(sys.argv) - 1) != 2):
    sys.exit('Usage: python convchallonge.py [url_to_bracket] [output_file]')

# Extract data from commandline arguments
bracket = sys.argv[1].partition('challonge.com/')[2]
outFile = sys.argv[2]

# Get tournament data using API
resp = requests.get(base + bracket + '.json')
resp.raise_for_status()
tourneyInfo = resp.json()['tournament']
date = tourneyInfo['started_at'][:10].split('-') # yyyy mm dd

# Trim leading 0's off the month and day
date[1] = date[1].lstrip('0')
date[2] = date[2].lstrip('0')

# Get participants of the tourney using API call
resp = requests.get(base + bracket + '/participants.json')
resp.raise_for_status()
players = resp.json()
playerDict = {}
# Build a simple dict of (id : name)
for player in players:
    player = player['participant']
    playerDict[player['id']]= player['name']

# Get match details and write each match to file
resp = requests.get(base + bracket + '/matches.json')
resp.raise_for_status()
matches = resp.json()
with open(outFile, 'w+') as f:
    for match in matches:
        match = match['match']
        m = {}
        m['p1Name'] = '"' + playerDict[match['player1_id']] + '"'
        m['p2Name'] = '"' + playerDict[match['player2_id']] + '"'
        scores = match['scores_csv']
        (p1Score, p2Score) =  ('', '')

        # Some extra logic here incase a match has a negative score
        # Challonge api returns scores as x-y so i can't just use split('-')
        # Because scores with negatives come out as 0--1 or -1-0
        # Should work if both numbers are negative.
        if (scores[0] == '-'):
            scores = scores[1:].split('-', 1)
            (p1Score, p2Score) = ('-' + scores[0], scores[1])
        else:
            scores = scores.split('-', 1)
            (p1Score, p2Score) = (scores[0], scores[1])
        m['p1Score'] = p1Score
        m['p2Score'] = p2Score

        # Write to file player 1's name, player 2's name, player 1's score,
        # playr 2's score, the day, month and year, all delimited by whitespace.
        line = '{} {} {} {} {} {} {}\n'
        f.write(line.format(m['p1Name'], m['p2Name'], m['p1Score'],
            m['p2Score'], date[2], date[1], date[0]))
