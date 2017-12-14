# pack into a tuple
def concat(*args, sep="/"):
    return sep.join(args)

print(concat("earth", "mars", "venus")) # 'earth/mars/venus'
print(concat("earth", "mars", "venus", sep="."))# 'earth.mars.venus'

def concat2(*args):
    return "/".join(args)

print(concat2("earth", "mars", "venus"))

# unpack a tuple param
args=[3,6]
print(list(range(*args)))# [3,4,5], unpack args first
