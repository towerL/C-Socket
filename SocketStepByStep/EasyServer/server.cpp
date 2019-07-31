#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <vector>

//三个命令：登入、登出和错误信息
enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

//包头
struct DataHeader {
    short dataLength; //数据长度
    short cmd; //命令
};

//包体 DataPackage
struct Login : public DataHeader
{
    Login() {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char UserName[32];
    char PassWord[32];
};

struct LoginResult : public DataHeader
{
    LoginResult() {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN;
        result = 0;
    }
    int result;
};

struct LogOut : public DataHeader
{
    LogOut() {
        dataLength = sizeof(LogOut);
        cmd = CMD_LOGOUT;
    }
    char UserName[32];
    
};

struct LogoutResult: public DataHeader
{
    LogoutResult() {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT;
        result = 0;
    }
    int result;
};

//创建一个动态数组，用来存储socket
std::vector<int> g_clients;

//处理进程
int processor(int _cSock) {
    char szRecv[4096] = {}; //缓冲，用于接收数据
    //5. 接收客户端的请求数据
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {
        printf("nLen <= 0, client has quit!\n");
        return -1;
    }
    //6. 处理请求
    switch (header->cmd) {
        case CMD_LOGIN:
        {
            recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
            Login* login = (Login*)szRecv;
            //接收客户端的登陆信息，这里要注意除去headerd部分，因此要加上数据偏移，并且读取范围要减去header的大小
            printf("receive message: CMD_LOGIN, data length:%d, username:%s, password: %s\n", login->dataLength, login->UserName, login->PassWord); //提示收到命令
            //假设用户输入正确（这里忽略用户名和密码是否正确的验证过程）
            LoginResult inret;
            //7. 向客户端发送数据send
            send(_cSock, (char*)&inret, sizeof(LoginResult), 0); //向客户端发送登陆结果
        }
            break;
        case CMD_LOGOUT:
        {
            recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
            LogOut* logout;
            printf("receive message: CMD_LOGIN, data length:%d, username:%s\n", logout->dataLength, logout->UserName);
            LogoutResult outret;
            //7. 向客户端发送数据send
            send(_cSock, (char*)&outret, sizeof(LogoutResult), 0);
        }
            break;
        default:
        {
            DataHeader header = {0, CMD_ERROR};
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
        }
            break;
    }
    return 0;
}

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
    
    //循环接收客户端数据
    while (true) {
        //fd_set类型成员包括socket数目和socket数组
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;
        
        //置空
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);
        
        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExp);
        
        //循环的加入数据
        for (int n = (int)g_clients.size() - 1; n >= 0 ; --n) {
            //将新加入的客户端放到可读集合中用于查询
            FD_SET(g_clients[n], &fdRead);
        }
        
        //select模型(nfds是指FED_SET集合中所有描述符（socket）的范围，而不是数量，即是所有文件描述符的最大值+1)
        int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
        if (ret < 0) {
            printf("select error, client has quit!\n");
            break;
        }
        
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //4. 等待客户端连接accept
            sockaddr_in clientAddr = {};
            socklen_t nAddrLen = sizeof(sockaddr_in);
            int _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); //套接字，收到客户端socket地址，返回socket地址的大小
            if (_cSock == -1) {
                perror("client socket error!\n");
            } else {
                g_clients.push_back(_cSock);
                printf("client socket success! socket = %d, IP = %s \n", _cSock, inet_ntoa(clientAddr.sin_addr)); //打印客户端socket和IP地址
            }
        }
        //循环调用进程处理函数
        for (int n = g_clients.size() - 1; n >= 0; --n){
            if (processor(g_clients[n]) == -1){ //如果客户端请求数据长度<=0则从客户端数组中h删除该socket
                auto it = find(g_clients.begin(), g_clients.end(), g_clients[n]);
                if (it != g_clients.end())
                    g_clients.erase(it);
            }
        }
        
    }
    
    //循环关闭数组中的socket
    for (int n = (int)g_clients.size() - 1; n >= 0 ; --n) {
        close(g_clients[n]);
    }
    
    //8. 关闭套接字close socket
    close(_sock);
    
    printf("Server has quit!\n");
    getchar();
    return 0;
}
