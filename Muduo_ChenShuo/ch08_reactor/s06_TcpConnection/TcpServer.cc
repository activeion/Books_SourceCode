// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpServer.h"

#include <muduo/base/Logging.h>
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <stdio.h>  // snprintf

using namespace muduo;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(listenAddr.toHostPort()),
    acceptor_(new Acceptor(loop, listenAddr)),
    started_(false),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
  if (!started_)
  {
    started_ = true;
  }

  if (!acceptor_->listenning())
  {
    loop_->runInLoop(
        boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
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
  TcpConnectionPtr conn(//创建了shared_ptr conn, use_count=1
      new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;//use_count=2
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1));
  conn->connectEstablished();
}//use_count=1

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnection [" << name_
           << "] - connection " << conn->name() << ", use_count="<<conn.use_count();
  //use_count=2, <1> connections_容器元素; <2> conn, 一个右值(shared_from_this()返回值)的左值引用
  size_t n = connections_.erase(conn->name());
  LOG_INFO << "After erase, use_count()=" << conn.use_count();//use_count=1, conn
  assert(n == 1); (void)n;
  loop_->queueInLoop(//TcpConnection& conn被存入boost::function对象中，
          //直到connectDestroyed()执行完毕后，swap到局部变量，导致ref--, 然后彻底销毁
      boost::bind(&TcpConnection::connectDestroyed, conn));
  LOG_INFO << "After bind, use_count()=" << conn.use_count();//use_count=2
  //use_count=2， <1> pendingFunctors_容器; <2> conn
}

