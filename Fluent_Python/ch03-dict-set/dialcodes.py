# BEGIN DIALCODES
# dial codes of the top 10 most populous countries
DIAL_CODES = [
        (86, 'China'),
        (91, 'India'),
        (1, 'United STates'),
        (62, 'Indonesia'),
        (55, 'Brazil'),
        (92, 'Pakistan'),
        (880, 'Bangladesh'),
        (234, 'Nigeria'),
        (7, 'Russia'),
        (81, 'Japan'),
    ]
d1 = dict(DIAL_CODES)   # <1>
print('d1:', d1.keys()) # 打印原始的key列表

d2 = dict(sorted(DIAL_CODES))   # <2>
print('d2:', d2.keys()) # 打印按照key排序的key列表

# lambda 参数列表 : 表达式
d3 = dict(sorted(DIAL_CODES, key=lambda x:x[1]))   # <3>
print('d3:', d3.keys()) # 打印按照value排序的key列表

# 用冒号:分割dict的key和value
country_code = { country:code for code,country in DIAL_CODES }
print(country_code)

upper = { code:country.upper() for country,code in country_code.items() if code < 66 }
print(upper)
