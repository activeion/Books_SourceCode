"""Download flags of top 20 countries by population

asyncio + aiottp version

Sample run::

    $ python3 flags_asyncio.py -s REMOTE
    EG VN IN TR RU ID US DE CN MX JP BD NG ET FR BR PH PK CD IR
    20 flags downloaded in 1.07s

"""
# BEGIN FLAGS_ASYNCIO
import asyncio
import async_timeout

import aiohttp  # <1>

from imported.flags import BASE_URL, save_flag, show, main  # <2>

"""
async def fetch(session, url):     
    with async_timeout.timeout(10):         
        async with session.get(url) as response:             
            return await response.text()  
async def main():     
    async with aiohttp.ClientSession() as session:         
        html = await fetch(session, 'http://python.org')         
        print(html)
"""

async def get_flag(cc):  # <3>
    url = '{}/{cc}/{cc}.gif'.format(BASE_URL, cc=cc.lower())
    async with aiohttp.ClientSession() as session:         
        with async_timeout.timeout(10):         
            async with session.get(url) as resp:             
                image = await resp.read()  # <5>
    return image


async def download_one(cc):  # <6>
    image = await get_flag(cc)  # <7>
    show(cc)
    save_flag(image, cc.lower() + '.gif')
    return cc


def download_many(cc_list):
    loop = asyncio.get_event_loop()  # <8>
    to_do = [download_one(cc) for cc in sorted(cc_list)]  # <9>
    wait_coro = asyncio.wait(to_do)  # <10>
    res, _ = loop.run_until_complete(wait_coro)  # <11>
    loop.close() # <12>

    return len(res)


if __name__ == '__main__':
    main(download_many)
# END FLAGS_ASYNCIO
