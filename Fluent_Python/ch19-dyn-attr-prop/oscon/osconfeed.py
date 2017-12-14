"""
osconfeed.py: Script to download the OSCON schedule feed

"""

# BEGIN OSCONFEED
from urllib.request import urlopen
import warnings
import os
import json

URL = 'http://www.oreilly.com/pub/sc/osconfeed'
JSON = 'data/osconfeed.json'


def load():
    if not os.path.exists(JSON):
        msg = 'downloading {} to {}'.format(URL, JSON)
        warnings.warn(msg)  # <1>
        with urlopen(URL) as remote, open(JSON, 'wb') as local:  # <2>
            local.write(remote.read())

    with open(JSON) as fp:
        return json.load(fp)  # <3>

# END OSCONFEED

if __name__ == '__main__':
    # BEGIN OSCONFEED_DEMO
    feed = load()  # <1>
    sorted(feed['Schedule'].keys())  # <2> ['conferences', 'events', 'speakers', 'venues']
    for key, value in sorted(feed['Schedule'].items()):
        print('{:3} {}'.format(len(value), key))  # <3> 1 conferences 484 events 357 speakers 53 venues
    print(feed['Schedule']['speakers'][-1]['name']) # <4> 'Carina C. Zona'
    print(feed['Schedule']['speakers'][-1]['serial']) # <5> 141590
    print(feed['Schedule']['events'][40]['name']) # 'There *Will* Be Bugs'
    print(feed['Schedule']['events'][40]['speakers']) # <6> [3471, 5199]
    # END OSCONFEED_DEMO
