
# BEGIN CLIP

def clip(text, max_len=80):
    """Return text clipped at the last space before or after max_len
    """
    end = None
    if len(text) > max_len:
        space_before = text.rfind(' ', 0, max_len)
        if space_before >= 0:
            end = space_before
        else:
            space_after = text.rfind(' ', max_len)
            if space_after >= 0:
                end = space_after
    if end is None:  # no spaces were found
        end = len(text)
    return text[:end].rstrip()

# END CLIP
print(clip('banana ', 6)) # 'banana'
print(clip('banana ', 7)) # 'banana'
print(clip('banana ', 5)) # 'banana'
print(clip('banana split', 6)) # 'banana'
print(clip('banana split', 7)) # 'banana'
print(clip('banana split', 10)) # 'banana'
print(clip('banana split', 11)) # 'banana'
print(clip('banana split', 12)) # 'banana split'


# 提取clip函数信息
print(clip.__defaults__)
print(clip.__code__)
print(clip.__code__.co_varnames)
print(clip.__code__.co_argcount)

# 更好的提取函数信息的方法，inspect模块
from inspect import signature

sig = signature(clip)
print(sig)
print(str(sig))

for name, param in sig.parameters.items():
    print(param.kind, ':', name, '=', param.default)

