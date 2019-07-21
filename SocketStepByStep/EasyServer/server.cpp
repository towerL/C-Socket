#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main() {
    //1. 建立一个socket(传入socket族，socket类型, 协议类型)
    int _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sock == -1) {
        perror("scoket error!");
        return 1;
    }
    
    //2. 绑定用于接受客户端连接的网络端口bind
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;      //协议族IPV4
    _sin.sin_port = htons(4567);    //端口号 host_to_net
    //_sin.sin_addr.s_addr = inet_addr("127.0.0.1");    //服务器绑定的ip地址
    _sin.sin_addr.s_addr= INADDR_ANY; //不限定访问该服务器的ip
    if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == -1) {
        perror("bind error!");
        return 1;
    } else {
        printf("bind success!\n");
    }
    
    //3. 监听网络端口listen
    if (listen(_sock, 5) == -1) { //套接字，最大允许连接数量
        perror("listen error!");
    } else {
        printf("listen success!\n");
    }
    
    //4. 等待客户端连接accept
    sockaddr_in clientAddr = {};
    socklen_t nAddrLen = sizeof(sockaddr_in);
    int _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); //套接字，收到客户端socket地址，返回socket地址的大小
    if (_cSock == -1) {
        perror("client socket error!");
    } else {
        printf("client socket success! socket = %d, IP = %s \n", _cSock, inet_ntoa(clientAddr.sin_addr)); //打印客户端socket和IP地址
    }
    
    //循环接收客户端数据
    char _recvBuf[128] = {};
    while (true) {
        //5. 接收客户端的请求数据
        int nLen = recv(_cSock, _recvBuf, 128, 0);
        if (nLen <= 0) {
            printf("client has quit!\n");
            break;
        }
        printf("receive message:%s\n", _recvBuf); //提示收到命令
        //6. 处理请求
        if (0 == strcmp(_recvBuf, "getName")) {
            //7. 向客户端发送数据send
            char msgBuf[] = "LIU";
            send(_cSock, msgBuf, sizeof(msgBuf) + 1, 0); //长度+1，将结尾符一并发送过去
        } else if (0 == strcmp(_recvBuf, "getAge")){
            //7. 向客户端发送数据send
            char msgBuf[] = "18";
            send(_cSock, msgBuf, sizeof(msgBuf) + 1, 0); //长度+1，将结尾符一并发送过去
        } else {
            //7. 向客户端发送数据send
            char msgBuf[] = "???";
            send(_cSock, msgBuf, sizeof(msgBuf) + 1, 0); //长度+1，将结尾符一并发送过去
        }
    }
    
    //8. 关闭套接字close socket
    close(_sock);
    
    printf("Server has quit!");
    getchar();
    return 0;
}
