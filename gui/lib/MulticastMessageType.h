#ifndef MULTICASTMESSAGETYPE_H
#define MULTICASTMESSAGETYPE_H

enum MulticastMessageType {
	kDefaultExistence,
	kDefaultReply,
	kUniqueJoin,
	kUniqueLeave,
	kUniqueClaim,
	kUniqueConfig,
	kUniqueConfigDelta,
	kUnknown
};

#endif // MULTICASTMESSAGETYPE_H
