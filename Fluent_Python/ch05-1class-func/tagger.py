
# BEGIN TAG_FUNC
def tag(name, *content, cls=None, **attrs):
    """Generate one or more HTML tags"""
    if cls is not None:
        attrs['class'] = cls
    if attrs:
        attr_str = ''.join(' %s="%s"' % (attr, value)
                           for attr, value
                           in sorted(attrs.items()))
    else:
        attr_str = ''
    if content:
        return '\n'.join('<%s%s>%s</%s>' %
                         (name, attr_str, c, name) for c in content)
    else:
        return '<%s%s />' % (name, attr_str)
# END TAG_FUNC

print(tag('br'))  # <1> '<br />'
print(tag('p', 'hello'))  # <2> '<p>hello</p>'
print(tag('p', 'hello', 'world')) #<p>hello</p> <p>world</p>
print( tag('p', 'hello', id=33)) # <3> '<p id="33">hello</p>'
print(tag('p', 'hello', 'world', cls='sidebar'))  # <4> <p class="sidebar">hello</p> <p class="sidebar">world</p>
print(tag(content='testing', name="img")) # <5> '<img content="testing" />'
print(tag(name="img", content='testing')) # <5> '<img content="testing" />'
print(tag("img", content='testing')) # <5> '<img content="testing" />'
my_tag = {'name': 'img', 'title': 'Sunset Boulevard', 'src': 'sunset.jpg', 'cls': 'framed'}
print(tag(**my_tag)) # <6> '<img class="framed" src="sunset.jpg" title="Sunset Boulevard" />'

# 下面的f函数的参数中，b为keyword-only arg
# 单独*后的所有参数均为keyword-only arg
def f(a, *, b): return a, b
print(f(1, b=2))


# 提取tag()函数的信息
from inspect import signature
sig = signature(tag)
for name, param in sig.parameters.items():
    print(param.kind, ':', name, '=', param.default)

my_tag = {'name':'img', 'title':'Sunset Boulevard', 'src':'sunset.jpg', 'cls':'framed'}
bound_args = sig.bind(**my_tag)
print(bound_args)
for name, value in bound_args.arguments.items():
    print(name, '=', value)



# 使用functools.partial冻结参数
# 类似于C++11的std::bind()函数？
print(tag) #查看tag函数的id
from functools import partial
picture = partial(tag, 'img', cls='pic-frame')
print(picture(src='wumpus.jpeg'))
print(picture)
print(picture.func)
print(picture.args)
print(picture.keywords)

