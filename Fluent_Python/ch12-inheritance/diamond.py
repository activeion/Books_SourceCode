class A:
    def ping(self):
        print('ping:', self)


class B(A):
    def pong(self):
        print('pong:', self)


class C(A):
    def pong(self):
        print('PONG:', self)


class D(B, C):

    def ping(self):
        super().ping() # A.ping(self) 绕过mro，直接到达任意一个父类
        print('post-ping:', self)

    def pingpong(self):
        self.ping()
        super().ping()
        self.pong()
        super().pong()
        C.pong(self)

if __name__ == '__main__':
    d = D()
    d.pong()
    C.pong(d)

    print(D.__mro__) # 经常查看每一个类的__mro__，分析调用优先级
    d.ping()

    d.pingpong()

