#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

//三个命令：登入、登出和错误信息
enum CMD {
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

//包头
struct DataHeader {
    short dataLength; //数据长度
    short cmd; //命令
};

//包体 DataPackage
struct Login {
    char UserName[32];
    char PassWord[32];
};

struct LoginResult {
    int result;
};

struct LogOut {
    char UserName[32];
    
};

struct LogoutResult {
    int result;
};

int main() {
    //1. 建立一个socket(传入socket族，socket类型, 协议类型)
    int _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sock == -1) {
        perror("socket error!");
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
    while (true) {
        DataHeader header = {}; //定义消息头结构体
        //5. 接收客户端的请求数据
        int nLen = recv(_cSock, (char*)&header, sizeof(header), 0);
        if (nLen <= 0) {
            printf("client has quit!\n");
            break;
        }
        printf("receive message:%d, data length:%d\n", header.cmd, header.dataLength); //提示收到命令
        //6. 处理请求
        switch (header.cmd) {
            case CMD_LOGIN:
            {
                Login login = {};
                recv(_cSock, (char*)&login, sizeof(Login), 0); //接收客户端的登陆信息（用户名+密码）
                //假设用户输入正确（这里忽略用户名和密码是否正确的验证过程）
                LoginResult inret = {0};
                //7. 向客户端发送数据send
                send(_cSock, (char*)&header, sizeof(DataHeader), 0); //向客户端发送消息头
                send(_cSock, (char*)&inret, sizeof(LoginResult), 0); //向客户端发送登陆结果
            }
            break;
            case CMD_LOGOUT:
            {
                LogOut logout = {};
                recv(_cSock, (char*)&logout, sizeof(LogOut), 0);
                LogoutResult outret = {1};
                //7. 向客户端发送数据send
                send(_cSock, (char*)&header, sizeof(DataHeader), 0);
                send(_cSock, (char*)&outret, sizeof(LogoutResult), 0);
            }
            break;
            default:
                header.cmd = CMD_ERROR;
                header.dataLength = 0;
                send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            break;
        }
        
    }
    
    //8. 关闭套接字close socket
    close(_sock);
    
    printf("Server has quit!");
    getchar();
    return 0;
}
