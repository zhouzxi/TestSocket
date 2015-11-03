/**********************************************************************
* 版权所有 (C)2015, Zhou Zhaoxiong。
*
* 文件名称：TestSocket.c
* 文件标识：无
* 内容摘要：测试socket从创建到结束的整个过程
* 其它说明：无
* 当前版本： V1.0
* 作   者： Zhou Zhaoxiong
* 完成日期： 20150608
*
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

// 宏定义
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#define BUFFER_LEN      1024
#define SERVERPORT      8999
#define IPADDR          "10.116.32.10"

// 重定义数据类型
typedef unsigned char  UINT8;
typedef signed   int   INT32;
typedef unsigned int   UINT32;
typedef          INT32 SOCKET;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr    *PSOCKADDR;

// 全局变量
UINT8 g_iStart  = 0;
UINT8 g_iFinish = 0;


/**********************************************************************
* 功能描述：主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期         版本号        修改人           修改内容
* -------------------------------------------------------------------
* 20150608        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 main()
{
    INT32  iRet     = -1;
    INT32  status   = 0;
    INT32  iTempVar = 0;
    UINT32 iLenth   = 0;

    UINT8 szBuffer[BUFFER_LEN] = {0};

    SOCKET      sockefd        = INVALID_SOCKET;
    SOCKET      clientSocketfd = {0};
    SOCKADDR_IN local_address  = {0};
    SOCKADDR_IN client_addrss  = {0};

    pthread_t tid = pthread_self();

    while (1)
    {
        // 创建socket
        close(sockefd);
        sockefd = socket(AF_INET, SOCK_STREAM, 0);
        if (INVALID_SOCKET == sockefd)
        {
            continue;
        }

        // 绑定指定IP地址和端口
        local_address.sin_family     = AF_INET;
        local_address.sin_port       = htons(SERVERPORT);
        local_address.sin_addr.s_addr= inet_addr(IPADDR);

        status   = SOCKET_ERROR;
        iTempVar = 1;

        if (setsockopt(sockefd, SOL_SOCKET, SO_REUSEADDR, (int *)&iTempVar, sizeof(iTempVar)) == -1)
        {
            printf("setsockopt SO_REUSEADDR FAILED! ERROR[%d] ERRORINFO[%s]\n", errno, strerror(errno));

            close(sockefd);;
            continue;    
        }

        status = bind(sockefd, (PSOCKADDR)&local_address, sizeof(SOCKADDR_IN));    // 绑定指定IP地址和端口
        if (status == SOCKET_ERROR)
        {
            close(sockefd);;
            printf("FAILED to bind ipaddr:%s!\n", IPADDR);

            continue; 
        }
        else
        {
            printf("Succeeded to bind ipaddr:%s, port:%d!\n", IPADDR, SERVERPORT);
            break;
        }
    }

    // 启动监听
    iRet = listen(sockefd, 10);
    if (iRet < 0)
    {
        printf("FAILED to stratup the listener!\n");

        return -1;
    }

    // 循环从监听队列中获取
    while (1)
    {
        iLenth = sizeof(client_addrss);
        clientSocketfd = accept(sockefd, (struct sockaddr *)&client_addrss, &iLenth);
        if (clientSocketfd == INVALID_SOCKET)
        {
            printf("The client socket is invalid!\n");

            continue;
        }
        printf("------------------------------------------\n");
        printf("Accept msg from SendMsgTool successfully!\n");

        memset(szBuffer, 0x00, sizeof(szBuffer));
        iLenth = recv(clientSocketfd, szBuffer, BUFFER_LEN-1, 0);
        if (iLenth <= 0)
        {
            printf("Server receive data failed! strerror=%s.\n", strerror(errno));

            continue; 
        }

        printf("Receive data: %s\n", szBuffer);
        if (0 == strncmp(szBuffer, "start test.\r\n", strlen("start test.\r\n")))
        {
            g_iStart = 1;

            // 执行相关操作, 打印消息
            UINT8 szCmd[1024] = {0};
            sprintf(szCmd, "This is a test for SOCKET!\n");

            printf("The command is: %s\n", szCmd);

            g_iFinish = 1;       // 将结束标识置为1
        }

        while (1)
        {
            if (1 == g_iFinish)
            {
                memset(szBuffer, 0x00, sizeof(szBuffer));
                sprintf(szBuffer, "task completed\r\n");
                if (send(clientSocketfd, szBuffer, strlen(szBuffer), 0) < 0)         // 发送结束本轮会话的消息
                {
                    printf("Send close msg failed!\n");
                }
                else
                {
                    printf("Send close msg OK!\n");
                }
                printf("------------------------------------------\n");

                g_iFinish = 0;
                g_iStart = 0;
                break;
            }
        }
    }

    return 0;
}
