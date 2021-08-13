# Web Server
本项目基于Linux采用C++11实现的轻量级Web服务器，并搭配日志记录，定时处理用户连接等功能，根据测试可见轻易达到上万的QPS


## Technical points
* 使用IO复用技术和线程池以及非阻塞的IO实现的Reactor模式的Web服务器
* 使用Epoll边沿触发的IO多路复用技术高效的处理事件
* 使用线程池充分发挥多核CPU的优势
* 基于时间轮高效统一的非活动连接
* 使用RAII方式管理资源
* 采用双缓冲区技术的异步日志系统，并支持等级过滤，自动回滚等功能
* 缓冲区底层采用标准库容器，实现能动态增长的缓冲区
* 使用状态机解析HTTP请求，支持长短连接

## 环境
* OS: CentOS 7
* Complier: g++ 4.8

## Build
	make

## Usage
	./webserver [-t 线程数] [-p 端口] [-l 日志等级] [-s 时间限制] [-r 保存日志] 

## 压力测试
* OS： CentOS 7
* 内存： 8G
* CPU： 4核

* 使用WebBench，模仿100、1000、10000个客户，测试60s
* 4个工作线程

![test](root/webserver.png)

* QPS： 1000000+

## TODO
* 数据库连接

## 致谢
	《Linux多线程服务端编程》陈硕

	


