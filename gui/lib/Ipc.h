#ifndef IPC_H
#define IPC_H

#define IPC_HOST "127.0.0.1"
#define IPC_PORT 24801

enum qIpcMessageType {
	kIpcHello,
	kIpcLogLine,
	kIpcCommand,
	kIpcShutdown,
};

enum qIpcClientType {
	kIpcClientUnknown,
	kIpcClientGui,
	kIpcClientNode,
};

extern const char* kIpcMsgHello;
extern const char* kIpcMsgLogLine;
extern const char* kIpcMsgCommand;
extern const char* kIpcMsgShutdown;

#endif // IPC_H
