#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>

void onConnection(const muduo::TcpConnectionPtr& conn)
{
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

/*
1. 创建TcpConnection
TcpConnection::TcpConnection
{

  channel_->setReadCallback(
      boost::bind(&TcpConnection::handleRead, this));
    //注意：这里没有使用shared_from_this(), 也不能使用
  ...
}
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, "#%d", nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toHostPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  TcpConnectionPtr conn(
      new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    //创建了shared_ptr conn, use_count=1
  connections_[connName] = conn;
    //use_count=2
  conn->setConnectionCallback(connectionCallback_); // void ::onConnection() --> "up"
  conn->setMessageCallback(messageCallback_);
  conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1));
  conn->connectEstablished();
}
//use_count=1

2. 使用和消亡
EventLoop::loop() ===> TcpConnection::handleRead()
void TcpConnection::handleRead()                                                                
{                                                                                          
    char buf[65536];                                                                    
    ssize_t n = ::read(channel_->fd(), buf, sizeof buf);                          
    if (n > 0) {//这里，use_count=1，因为this被存入了TcpServer::connections_容器
        // shared_from_this()返回一个右值，这个右值被一个conn左值引用绑定，延长了生命周期
        messageCallback_(shared_from_this(), buf, n); // use_count=2: <1> 右值 <2> connections_
            ===> void onMessage(TcpConnectionPtr& conn, const char* data, ssize_t len)// use_count=2; <1> conn <2> connections_
        //messageCallback_()返回后，use_count依然等于1，因为TcpServer::connections_容器中该元素依然存在
    } else if (n == 0) {
        handleClose();
            ===> closeCallback_(shared_from_this()) // use_count=2; <1> 右值 <2> connections_
                ====> TcpServer::removeConnection(const TcpConnectionPtr& conn) //use_count=1 <1> 右值
                    ===> loop_->queueInLoop(boost::bind(connectionDestroyed, conn)) // use_count=2 <1> 右值 <2> bind参数
        //handleClose()返回, use_count=1, <1> pendingFunctors_
        ===> functors.swap(pendingFunctors_);// functors析构, TcpConnection最终消亡
    } else {
        handleError();
    }
    ...
}

*/
void onMessage(const muduo::TcpConnectionPtr& conn,
               const char* data,
               ssize_t len)
{
  printf("onMessage(): received %zd bytes from connection [%s], conn.use_count()=%d\n",
         len, conn->name().c_str(), conn.use_count()); // use_count=2
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  muduo::InetAddress listenAddr(9981);
  muduo::EventLoop loop;

  muduo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
}
