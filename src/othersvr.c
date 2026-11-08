/*实现othersvr里的函数*/

#include "othersvr.h"
#include "DNS.h"

struct DNSHeader header;
struct DNSQuery query;
struct DNSRR rr[10];
struct sockaddr_in clientAddr;	//记录传输中的客户端地址
unsigned char dnsmessage[1024];//报文
unsigned char* rr_ptr;			//记录rr的位置
unsigned char* get_rr_ptr;		//用于getRR的指针
char* filename;					//文件名
int sockettcp;					//套接字标识符
int err;						//记录返回值
int len_header_query = 0;   	//记录报文中资源记录之前部分的长度

void initSocket(const char* svr, const char* _filename) {
    filename = _filename;
    // 初始化TCP套接字
    sockettcp = socket(AF_INET, SOCK_STREAM, 0); // TCP
    if (sockettcp < 0) {
        printf("create TCP socket failed: %d\n", errno);
        exit(0);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(svr);

   // 绑定本地地址和端口
    if (bind(sockettcp, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Failed to bind local address");
        exit(1);
    }

    // 监听连接请求
    if (listen(sockettcp, 1) == -1) {
        perror("Failed to listen for connections");
        exit(1);
    }
    // 接受连接请求
    socklen_t clientLen = sizeof(clientAddr);
    sockettcp = accept(sockettcp, (struct sockaddr*)&clientAddr, &clientLen);
    if (sockettcp == -1) {
        perror("Failed to accept connection");
        exit(1);
    }

    printf("Waiting for connections...\n");
}

int containString(const unsigned char* dname, const unsigned char* rname, const unsigned char type)
{
    int len1 = strlen(dname);
    int len2 = strlen(rname);
    int i = len1 - 1, j = len2 - 1;
    if(type == 'N'||type=='P'){//NS特殊，只匹配一部分
        for(;; i--,j--) //自后向前遍历
        {
            if(j < 0)//rname读完,表示每一位都匹配上
            {
                return 1;
            }
            if(dname[i] != rname[j])//某一位未匹配上
                return -1;
        }
    }
    else
    {
        if(strcmp(dname, rname) == 0)
        {
            return 1;
        }
        return -1;
    }
}

void setRR()//匹配，
{
    unsigned char temp_rr[256];
    rr_ptr = getMessage(&header, &query, dnsmessage, &len_header_query);
    get_rr_ptr = rr_ptr;
    memset(rr_ptr, 0, sizeof(dnsmessage) - len_header_query);//清空报文中的rr部分
    unsigned char* ptr = dnsmessage;
    ptr += 6;
    *((unsigned short*)ptr) = 0;//报头的资源记录数置零
    ptr += 2;
    *((unsigned short*)ptr) = 0;
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        printf("the file cannot be opened: %d\n", errno);
        exit(0);
    }
    unsigned char dname[128];
    memset(dname, 0, sizeof(dname));
    unsigned char* temp_ptr = query.name;
    int flag, i, num = 0;
    for(;;)//将query.name转换成标准的域名格式
    {
        flag = (int)temp_ptr[0];
        for(i = 0; i < flag; i++)
        {
            dname[i + num] = temp_ptr[i + 1];
        }
        temp_ptr += (flag + 1);
        if((int)temp_ptr[0] == 0)
            break;
        dname[flag + num] = '.';
        num += (flag + 1);
    }
    while(fgets(temp_rr, sizeof(temp_rr), fp) != NULL)//逐行查询
    {
        unsigned char rname[128];//记录一条资源记录中第一个空格前的部分
        unsigned char type;//记录第二个空格后的字符，也就是RR类型的首字母
        memset(rname, 0, sizeof(rname));
        int len = strlen(temp_rr);
        for(i = 0; i < len; i++)
        {
            if(temp_rr[i] == ' ')
                break;
        }
        memcpy(rname, temp_rr, i);
        int numofspace = 0;
        for(i = 0; i < len; i++)
        {
            if(temp_rr[i] == ' ')
                numofspace++;
            if(temp_rr[i] == ' ' && numofspace == 2)
                break;
        }
        type = temp_rr[i + 1];
        if(containString(dname, rname, type) == 1)
        {
            addRR(temp_rr, rname);
        }
        memset(temp_rr, 0, sizeof(temp_rr));
    }
    err = fclose(fp);
    if(err == EOF)
    {
        printf("The file close failed: %d\n", errno);
        exit(0);
    }
}

void addRR(const unsigned char* str, const unsigned char* rname)
{
    unsigned char buf[128];
    unsigned char* ptr = dnsmessage;
    ptr += 6;
    *((unsigned short*)ptr) = htons(htons(*((unsigned short*)ptr)) + 1);//报头的资源记录数加1 AnswerNum+1
    ptr = buf;
    char *pos;
    int n, len = 0;//len记录域名的长度
    pos = (char*)rname;
    /*将域名存到buf中，buf中存储每个域的长度和内容
    比如当前域是edu.cn，存到buf中就变成了3edu2cn0
    ,0表示结尾*/
    for(;;)
    {
        n = strlen(pos) - (strstr(pos , ".") ? strlen(strstr(pos , ".")) : 0);//计算pos开始到第一个.之间的字符数
        *ptr ++ = (unsigned char)n;
        memcpy(ptr , pos , n);
        len += n + 1;
        ptr += n;
        if(!strstr(pos , "."))
        {
            *ptr = (unsigned char)0;
            ptr ++;
            len += 1;
            break;
        }
        pos += n + 1;
    }
    memcpy(rr_ptr, buf, len);//转换后域名3edu2cn0放进rr_ptr里
    rr_ptr += len;
    pos = (char*)str;
    pos += (len + 2);
    int flag = 0;
    /*因为只考虑A,NS,MX,CNAME四种查询类型，所以只做了匹配第一个字母的简单处理*/
    switch(pos[0])
    {
    case'A'://A
    {
        *((unsigned short*)rr_ptr) = htons(1);
        rr_ptr += 2;
        pos += 2;
        flag = 1;
        break;
    }
    case'N'://NS
    {
	unsigned char* _ptr = dnsmessage;
        _ptr += 6;
        *((unsigned short*)_ptr) = htons(htons(*((unsigned short*)_ptr)) - 1);
        _ptr += 2;
        *((unsigned short*)_ptr) = htons(htons(*((unsigned short*)_ptr)) + 1);
        *((unsigned short*)rr_ptr) = htons(2);
        rr_ptr += 2;
        pos += 3;
        break;
    }
    case'C'://CNAME
    {
        *((unsigned short*)rr_ptr) = htons(5);
        rr_ptr += 2;
        pos += 6;
        break;
    }
    case'M'://MX
    {
        *((unsigned short*)rr_ptr) = htons(15);
        rr_ptr += 2;
        pos += 3;
        flag = 2;
        break;
    }
    case'P'://PTR的类型
    {
        *((unsigned short*)rr_ptr) = htons(12);
        rr_ptr += 2;
        pos += 4;
        //这里写点啥？
        break;
    }
    }
    *((unsigned short*)rr_ptr) = htons(1);
    rr_ptr += 2;
    *((unsigned short*)rr_ptr) = htonl(0);
    rr_ptr += 4;
    len = strlen(pos);
    len = len - 2;//len - 2是因为从文件中读取的字符串最后两位是回车加换行
    if (flag == 1)
    {
        *((unsigned short*)rr_ptr) = htons(4);
        rr_ptr += 2;
        struct in_addr addr;
        char ip[32];
        memset(ip, 0, sizeof(ip));
        memcpy(ip, pos, len);
        inet_aton(ip, &addr);
        *((unsigned long*)rr_ptr) = addr.s_addr;
        rr_ptr += 4;
    }
    else if(flag == 2)
    {
	*((unsigned short*)rr_ptr) = htons(len);
        rr_ptr += 2;
        memcpy(rr_ptr, pos - 3, 2);
        rr_ptr += 2;
        *rr_ptr = (unsigned char)len;
        rr_ptr += 1;
        memcpy(rr_ptr, pos, len);
        rr_ptr += len;
        memset(rr_ptr, 0, 1);
        rr_ptr++;
    }
    else
    {
        *((unsigned short*)rr_ptr) = htons(len);
        rr_ptr += 2;
        memcpy(rr_ptr, pos - 1, len + 1);
        rr_ptr += (len + 1);
    }
}

void setAddRR()
{
    rr_ptr = getMessage(&header, &query, dnsmessage, &len_header_query);
    rr_ptr = getRR(rr, &header, rr_ptr);
    rr_ptr++;
    int i, j;
    for(j = 0; j < header.answerNum; j++)
    {
        if(rr[i].type == 15)//找到MX对应data对应的IP地址
        {
            unsigned char temp_rr[256];
            unsigned char type;//记录第二个空格后的字符，也就是RR类型的首字母
            FILE *fp;
            fp = fopen(filename, "r");
            if(fp == NULL)
            {
                printf("the file cannot be opened: %d", errno);
                exit(0);
            }
            while(fgets(temp_rr, sizeof(temp_rr), fp) != NULL)//逐行查询
            {
                unsigned char rname[128];//记录一条资源记录中第一个空格前的部分
                memset(rname, 0, sizeof(rname));
                int len = strlen(temp_rr);
                for(i = 0; i < len; i++)
                {
                    if(temp_rr[i] == ' ')
                        break;
                }
                memcpy(rname, temp_rr, i);
                int numofspace = 0;
                for(i = 0; i < len; i++)
                {
                    if(temp_rr[i] == ' ')
                        numofspace++;
                    if(temp_rr[i] == ' ' && numofspace == 2)
                        break;
                }
                type = temp_rr[i + 1];
                if(containString(rr[j].rdata, rname, type) == 1)
                {
                    addRR(temp_rr, rname);
                    unsigned char* ptr = dnsmessage;
                    ptr += 6;
                    /*因为添加additional rr也是用的添加RR的函数，所以
                    需要报头的资源记录数减1，然后附加资源记录数加1*/
                    *((unsigned short*)ptr) = htons(htons(*((unsigned short*)ptr)) - 1);
                    ptr += 4;
                    *((unsigned short*)ptr) = htons(htons(*((unsigned short*)ptr)) + 1);
                }
                memset(temp_rr, 0, sizeof(temp_rr));
            }
            err = fclose(fp);
            if(err == EOF)
            {
                printf("The file close failed: %d", errno);
                exit(0);
            }
            break;
        }
    }
}

void receivefromServer(int flag)//从其他服务器接收, flag表示是否要记录客户端地址
{
	memset(dnsmessage, 0, sizeof(dnsmessage));
	switch(flag)
	{
		case 1:////clientAddr 记录TCP传输中的客户端地址
		{
            unsigned char temp[1024];
            memset(temp,0,sizeof(temp));
			int len = sizeof(clientAddr);
            err = recv(sockettcp, temp, sizeof(temp), 0);
            memcpy(dnsmessage,temp+2,sizeof(temp)-2);
            break;
		}
	}
    if (err < 0) {
            printf("TCP socket receive failed: %d\n", errno);
            exit(0);
    }
}

void sendtoSvr(const unsigned char* svr, int flag)//向其他服务器发送,flag表示是否是向记录的地址发送
{
	switch(flag)
	{
		case 1:
		{
            unsigned char* ptr = dnsmessage;
            ptr += 2;
            if (*((unsigned short*)ptr) == htons(0x0080))
            {
                *((unsigned short*)ptr) = htons(0x8080);
            }
            unsigned char temp[1024];
            memset(temp, 0, sizeof(temp));
            *((unsigned short*)temp) = htons(512);
            memcpy(temp + 2, dnsmessage, sizeof(dnsmessage) - 2);
            err = send(sockettcp, temp, 514, 0);
            // int len = sizeof(dnsmessage);
			// err = send(sockettcp, dnsmessage, len+14, 0);
        }
	}
	if(err <= 0)
    {
        printf("send question to next dns failed: %d\n", errno);
        exit(0);
    }
}

void iteration()
{
    printf("\nIteration is working\n");

        sendtoSvr("", 1);


}


void process()
{
	while(1)
    {
        receivefromServer(1);//从其他服务器接收, flag表示是否要记录客户端地址,flag=1表示client的地址
        setRR();//决定报文的type
        setAddRR();//构建报文
        if(header.tag == 0x0080)//iteration
        {
            iteration();//迭代
        }

    }
}
