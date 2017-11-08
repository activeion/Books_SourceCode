import doctest
import importlib
import glob

TARGET_GLOB = 'class/sentence_*.py'

def main(argv):
    verbose = '-v' in argv
    for module_file_name in sorted(glob.glob(TARGET_GLOB)):
        module_name = module_file_name.replace('.py', '')
        module_name = module_name.replace('/', '.')
        module = importlib.import_module(module_name)
        try:
            cls = getattr(module, 'Sentence')
        except AttributeError:
            continue
        test(cls, verbose)


'''
==============================
Tests for a ``Sentence`` class
==============================
'''
def test(Sentence, verbose=False):
    # A ``Sentence`` is built from a ``str`` and allows iteration word-by-word.
    s = Sentence('The time has come')
    print(s) # Sentence('The time has come')
    print(list(s)) # ['The', 'time', 'has', 'come']
    it = iter(s)
    print(next(it)) # 'The'
    print(next(it)) # 'time'
    print(next(it)) # 'has'
    print(next(it)) # 'come'
    # print(next(it)) # Traceback (most recent call last): ...  StopIteration

    # Any punctuation is skipped while iterating::
    s = Sentence('"The time has come," the Walrus said,')
    print(s) # Sentence('"The time ha... Walrus said,')
    print(list(s)) # ['The', 'time', 'has', 'come', 'the', 'Walrus', 'said']

    # White space including line breaks are also ignored::
    s = Sentence('''"The time has come," the Walrus said, "To talk of many things:"''')
    print(s) # Sentence('"The time ha...many things:"')
    print(list(s)) # ['The', 'time', 'has', 'come', 'the', 'Walrus', 'said', 'To', 'talk', 'of', 'many', 'things']

    # Accented Latin characters are also recognized as word characters::
    s = Sentence('Agora vou-me. Ou me vão?')
    print(s) # Sentence('Agora vou-me. Ou me vão?')
    print(list(s)) # ['Agora', 'vou', 'me', 'Ou', 'me', 'vão']


if __name__ == '__main__':
    import sys
    main(sys.argv)
