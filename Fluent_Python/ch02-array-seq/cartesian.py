colors = ['black', 'white']
sizes = ['S', 'M', 'L']
tshirts = [(color, size) for color in colors for size in sizes]
print(tshirts)
for color in colors:
    for size in sizes:
        print((color, size))

tshirts = [(color,size) for size in sizes for color in colors]
print(tshirts)
