#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat May  2 14:43:56 2020

@author: jizh
"""

lax_coordinates=(33.9425, -118.408056)
city, year, pop, chg, area = ('Tokyo', 2003, 32450, 0.66, 8014)# 元组拆包

traveler_ids = [('USA', '31195855'), ('BRA', 'CE342567'), ('ESP', 'XDA205856')]
for passport in sorted(traveler_ids):
    print('%s/%s' % passport, end='\t') # 元组拆包
    print(f'{passport}')

print('------')
for country, _ in traveler_ids: # _ 占位符
    print(country)

