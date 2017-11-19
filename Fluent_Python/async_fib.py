# -*- coding: utf-8 -*-
import asyncio
import random

async def smart_fib(n):
	index = 0
	a = 0
	b = 1
	while index < n:
		sleep_secs = random.uniform(0, 0.2)
		await asyncio.sleep(sleep_secs)
		print('Smart one think {} secs to get {}'.format(sleep_secs, b))
		a, b = b, a + b
		index += 1

async def stupid_fib(n):
	index = 0
	a = 0
	b = 1
	while index < n:
		sleep_secs = random.uniform(0, 0.4)
		await asyncio.sleep(sleep_secs)
		print('Stupid one think {} secs to get {}'.format(sleep_secs, b))
		a, b = b, a + b
		index += 1

if __name__ == '__main__':
	loop = asyncio.get_event_loop()
	tasks = [
		asyncio.ensure_future(smart_fib(10)),
		asyncio.ensure_future(stupid_fib(10)),
	]
	loop.run_until_complete(asyncio.wait(tasks))
	print('All fib finished.')
	loop.close()
