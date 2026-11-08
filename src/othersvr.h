#ifndef OTHERSVR_H_INCLUDED
#define OTHERSVR_H_INCLUDED

void initSocket(const char* svr,
			const char* _filename);					//初始化套接字，记录文件名
int containString(const unsigned char* dname,
			const unsigned char* rname,
			const unsigned char type);				//域名匹配
void setRR();										/*查询文件中存储的资源记录,查询到符合要求
													的则调用addRR函数，将资源记录加入到报文中*/
void addRR(const unsigned char* str,
		const unsigned char* rname);				//添加资源记录到报文中
void setaddrr();									//添加附加资源记录到报文中
void receivefromServer(int flag);							/*从其他服务器接收, flag表示是否要记录客户端地址
													*/
void sendtoSvr(const unsigned char* svr, int flag);	/*向其他服务器发送
													flag表示是否是向记录的地址发送*/
void iteration();									//迭代

void process();										//总运行过程

#endif // OTHERSVR_H_INCLUDED
