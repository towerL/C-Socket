//
//  client.cpp
//  SocketStepByStep
//
//  Created by 刘君妍 on 2019/7/19.
//  Copyright © 2019 tower. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_ERROR -1

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

int main() {
    //1. 建立一个socket
    int _sock = socket(AF_INET, SOCK_STREAM, 0); //与服务器端不同，第三个参数无需声明使用TCP连接
    if (_sock == SOCKET_ERROR) {
        printf("socket build error!\n");
    } else {
        printf("socket build success!\n");
    }
    
    //2. 连接服务器
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (ret == SOCKET_ERROR) {
        printf("connect error!\n");
    } else {
        printf("connect success!\n");
    }
    
    char cmdBuf[128] = {}; //大小与服务器端匹配，防止溢出
    while (true) {
        //3. 用户输入请求命令
        scanf("%s", cmdBuf);
        //4. 处理请求命令
        if (0 == strcmp(cmdBuf, "exit")) {
            printf("receive quit message!");
            break;
        } else if (0 == strcmp(cmdBuf, "login")){
            //5. 向服务器端发送请求
            Login login;
            strcpy(login.UserName, "liu");
            strcpy(login.PassWord, "1234");
            send(_sock, (const char*)&login, sizeof(Login), 0);
            //6. 接收服务器返回数据
            LoginResult loginRet = {};
            recv(_sock, (char*)&loginRet, sizeof(LoginResult), 0);
            printf("LoginResult: %d\n", loginRet.result);
            
        } else if (0 == strcmp(cmdBuf, "logout")) {
            //5. 向服务器端发送请求
            LogOut logout;
            strcpy(logout.UserName, "liu");
            send(_sock, (const char*)&logout, sizeof(LogOut), 0);
            //6. 接收服务器返回数据
            LogoutResult logoutRet = {};
            recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
            printf("LogoutResult: %d\n", logoutRet.result);
            
        } else {
            printf("unsupported command!");
        }
    }
    
    //7. 关闭套接字close socket
    close(_sock);
    
    printf("client has quit!");
    getchar();
    return 0;
}
