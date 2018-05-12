// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpConnection.h"

#include <muduo/base/Logging.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
  LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
            << " fd=" << sockfd;
  channel_->setReadCallback(
      boost::bind(&TcpConnection::handleRead, this));
  channel_->setWriteCallback(
      boost::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(
      boost::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(
      boost::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading();
  //这里的shared_from_this()返回一个右值. 
  //connectionCallback_=boost::bind(TcpServer::connectionCallback_, this)=::onConnection(...)
  //因此右值被绑定到::onConnection的形参cont TcpConnectionPtr& conn这个左值引用上了。
  //注意：右值并没有被放入函数对象connectionCallback_中去！
  connectionCallback_(shared_from_this());//void ::onConnection(const TcpConnectionPtr& conn)
  //调用结束之后，程序运行到这里，this的引用计数仅仅为1，即pendingFunctors容器的成员boost::function的参数
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnected);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());

  loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::handleRead()
{
  char buf[65536];
  ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
  if (n > 0) {
    messageCallback_(shared_from_this(), buf, n);
  } else if (n == 0) {
    handleClose();
  } else {
    handleError();
  }
}

void TcpConnection::handleWrite()
{
}

void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpConnection::handleClose state = " << state_;
  assert(state_ == kConnected);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  channel_->disableAll();
  // must be the last line
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
