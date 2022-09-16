#pragma warning(disable: 4786)
//服务端
#include <WINSOCK2.H>   
#include <stdio.h>     
#include <string>
#include <iostream>
#include<Windows.h>
#include<process.h>
#include <malloc.h>
#include <map>
#include <vector>
#include "Server.h"
#include "Command.h"
using namespace std;

#pragma warning( disable : 4996 )

//服务端数据库数组的个数
//端口号                    
#define PORT           5150    
//发送消息的最大字节长度
#define MSGSIZE        1024    

#pragma comment(lib, "ws2_32.lib")  

#define MAXCLIENTNUM 16

//服务器需要一个存放资源的db数组，每个客户端一连接，
//就创建一个Client，并且让它与对应的数据库db[i]绑定
//db中是一个字典构成的数组，让对应的Client存放key-value
//客户端输入命令，服务器对命令进行拆分，在自己的命令
//集合中查询命令是否合法后，相应的命令。
//客户端若执行set命令，则将key-value存储在db的字典数组中，最后返回响应给客户端。
typedef void (*CommandFun) (Server*, Client*, string, string&,bool&);
typedef map<string, CommandFun> mymap;
typedef Skiplist<string, string> myDB;
//创建服务端
Server* server = new Server();


//set命令
void setCommand(Server* server, Client* client, string key, string& value, bool& flag) {
	flag=client->db.insert_element(key, value);
}

//get命令
void getCommand(Server* server, Client* client, string key, string& value, bool& flag) {
	flag = client->db.search_element(key, value);

}

//del命令
void delCommand(Server* server, Client* client, string key, string& value, bool& flag) {
	flag=client->db.delete_element(key);
	
}

//load命令
void loadCommand(Server* server, Client* client, string key, string& value, bool& flag) {
	client->db.load_file();
}
//dump命令
void dumpCommand(Server* server, Client* client, string key, string& value, bool& flag) {
	client->db.dump_file();
}

//初始化数据库
void initDB() {
	//cout<<"init db..."<<endl;
	cout << "Redis by Stack start-up..." << endl;
}

//初始化命令库
void initCommand(Server*& server) {
	(server->Commands).insert(pair<string, CommandFun>("set", &setCommand));
	

	(server->Commands).insert(pair<string, CommandFun>("get", &getCommand));
	/*	Client*c1=new Client();
		mymap::iterator it;
		it=server->Commands.find("get");
		CommandFun cmd=it->second;
		cmd(server,c1);*/
	(server->Commands).insert(pair<string, CommandFun>("del", &delCommand));
	(server->Commands).insert(pair<string, CommandFun>("load", &loadCommand));
	(server->Commands).insert(pair<string, CommandFun>("dump", &dumpCommand));
}


//初始化服务端
void initServer(Server*& server) {
	//创建客户端的套接字
	server->sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//初始化数据库
	initDB();

	//初始化命令库
	initCommand(server);

	//绑定
	(server->local).sin_family = AF_INET;
	(server->local).sin_port = htons(PORT);
	(server->local).sin_addr.s_addr = htonl(INADDR_ANY);
	bind(server->sListen, (struct sockaddr*)&(server->local), sizeof(SOCKADDR_IN));

	//监听
	//1表示等待连接队列的最大长度
	listen(server->sListen, 1);
}


//创建工作线程
DWORD WINAPI WorkerThread(LPVOID lpParam) {
	Client* c = (Client*)lpParam;
	char szMessage[MSGSIZE];
	int ret;
	while (true&&c->sClient) {
		//ret是发送消息的字节长度
		//szMessage是发送消息的内容
		
		ret = recv(c->sClient, szMessage, MSGSIZE, 0);
		if (ret <= 0)return 0;
		szMessage[ret] = '\0';

		//printf("Received [%d bytes]: '%s'\n", ret, szMessage); 

		//类型转换，方便比较
		string message(szMessage);
		//对用户发送的消息进行判断
		//cout<<"else hh"<<endl;
		//对发送的消息进行分割，查看是否符合get/set命令的格式
		//分隔符
		Command C(message);
		C.split_command();
		//执行set命令
		if (C._arg[0] == "set") {
			string key = C._arg[1];
			string value = C._arg[2];
			bool flag = true;
			mymap::iterator it;
			it = server->Commands.find("set");

			CommandFun cmd = it->second;
			cmd(server, c, key, value,flag);
			//向客户端发送数据
			char  _char_array[] = "OK";
			char* str = _char_array;
			send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			continue;

			//执行get命令
		}
		if (C._arg[0] == "get") {
			//符合命令格式

			//cout << "ok" << endl;
			//获得键值
			string key = C._arg[1];
			string value = "";
			bool flag = true;
			//cout << C._arg[0] << " " << C._arg[1] << endl;

			mymap::iterator it;
			it = server->Commands.find("get");
			CommandFun cmd = it->second;
			//向客户端发送数据
			//char *str;
			//int len=value.length();
			//str=(char*)malloc((len+1)*sizeof(char));
			//value.copy(str,len,0);
			cmd(server, c, key, value, flag);
			if (flag) {
				char* str = new char[strlen(value.c_str())];
				strcpy(str, value.c_str());
				send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			}
			else {
				char  _char_array[] = "(nil)";
				char* str = _char_array;
				send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			}
			continue;
		}
		if (C._arg[0] == "del") {
			//符合命令格式
			//cout << "ok" << endl;
			//获得键值
			string key = C._arg[1];
			string value = "";
			bool flag = true;
			//cout << C._arg[0] << " " << C._arg[1] << endl;
			mymap::iterator it;
			it = server->Commands.find("del");
			CommandFun cmd = it->second;	
			//向客户端发送数据
			//char *str;
			//int len=value.length();
			//str=(char*)malloc((len+1)*sizeof(char));
			//value.copy(str,len,0);
			cmd(server, c, key, value, flag);
			if (flag) {
				string sendMessage = "(integer) 1";
				char* str = new char[strlen(sendMessage.c_str())];
				strcpy(str, sendMessage.c_str());
				send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			}
			else {
				string sendMessage = "(integer) 0";
				char* str = new char[strlen(sendMessage.c_str())];
				strcpy(str, sendMessage.c_str());
				send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			}
			continue;
		}
		if (C._arg[0] == "load") {
			//符合命令格式
			//获得键值
			string key = "";
			string value = "";
			bool flag = true;
			//cout << C._arg[0] << " " << C._arg[1] << endl;
			mymap::iterator it;
			it = server->Commands.find("load");
			CommandFun cmd = it->second;
			cmd(server, c, key, value,flag);
			//向客户端发送数据
			//char *str;
			//int len=value.length();
			//str=(char*)malloc((len+1)*sizeof(char));
			//value.copy(str,len,0);
			string sendMessage = "(integer) 1";
			char* str = new char[strlen(sendMessage.c_str())];
			strcpy(str, sendMessage.c_str());
			send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			continue;
		}
		if (C._arg[0] == "dump") {
			//符合命令格式
			//获得键值
			string key = "";
			string value = "";
			bool flag = true;
			//cout << C._arg[0] << " " << C._arg[1] << endl;
			mymap::iterator it;
			it = server->Commands.find("dump");
			CommandFun cmd = it->second;
			cmd(server, c, key, value,flag);
			//向客户端发送数据
			//char *str;
			//int len=value.length();
			//str=(char*)malloc((len+1)*sizeof(char));
			//value.copy(str,len,0);
			string sendMessage = "(integer) 1";
			char* str = new char[strlen(sendMessage.c_str())];
			strcpy(str, sendMessage.c_str());
			send(c->sClient, str, strlen(str) + sizeof(char), NULL);
			continue;
		}
	}
	return 0;
}
int main()
{

	WSADATA wsaData;
	int iaddrSize = sizeof(SOCKADDR_IN);
	Client* client;

	DWORD dwThreadId;


	//初始化Windows套接字库
	WSAStartup(0x0202, &wsaData);

	//初始化服务端
	initServer(server);

	//获得线程的句柄
	HANDLE hThread[MAXCLIENTNUM];

	for (int i = 0; i < MAXCLIENTNUM; i++) {
		//等待客户端连接
		Client* client = new Client();
		client->sClient = accept(server->sListen, (struct sockaddr*)&(client->my_client), &iaddrSize);

		/*printf("Accepted client:%s:%d\n", inet_ntoa((client->my_client).sin_addr),
			ntohs((client->my_client).sin_port)); */
		cout << "Accepted client:" << inet_ntoa((client->my_client).sin_addr) << " : "
			<< ntohs((client->my_client).sin_port) << endl;

		//将客户端与服务端中对应的数据库连接,后续可能会针对此接口进行管理。
		server->DB[i] = &(client->db);

		//创建工作线程
		hThread[i] = CreateThread(NULL, 0, WorkerThread, client, 0, NULL);
	}

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	//关闭套接字  
	closesocket(server->sListen);
	//delete[]server->DB;
	cout << "ALL FINISHED" << endl;
	return 0;
}