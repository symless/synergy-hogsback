#ifndef MULTICASTMESSAGETYPE_H
#define MULTICASTMESSAGETYPE_H

enum MulticastMessageType {
        kDefaultExistence, // new screen multicast this msg looking for server
        kDefaultReply, // all servers reply with this msg along with if they are active
        kUniqueJoin, // new screen joined the unique group
	kUniqueLeave,
	kUniqueClaim,
	kUniqueConfig,
	kUniqueConfigDelta,
	kUnknown
};

#endif // MULTICASTMESSAGETYPE_H
