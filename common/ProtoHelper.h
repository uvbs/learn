#ifndef __ProtoHelper_H
#define __ProtoHelper_H

#include "common.h"
#include "utility.h"
#include "YYError.h"
#include "Marshallable.h"

class ProtoHelper {
public:
	enum {
		_RES_SUCCESS = 200,


		HEAD_LEN = 10,

		LBS_SVID = 1,
		IMTRANS_SVID = 36, //imtransfer???
		AUDIO_SVID = 200,

		// statistics sdk
		PExchangeKeyLBSResURI = (8 << 8 | LBS_SVID),
		YY_STAT_CRASH_URI = (2006 << 8 | AUDIO_SVID),
		YY_STAT_EVENT_URI = (2005 << 8 | AUDIO_SVID),
		YY_STAT_FEEDBACK_URI = (2007 << 8 | AUDIO_SVID),
		YY_STAT_INFO_URI = (2004 << 8 | AUDIO_SVID),
		YY_STAT_ACK_URI = (2008 << 8 | AUDIO_SVID),

		// Call Manager
		PAlertingURI = ( 27 << 8 | AUDIO_SVID ),
		PCheckConnectionURI = (36 << 8 | AUDIO_SVID),
		PCheckConnectionAckURI = (37 << 8 | AUDIO_SVID),
		PContractAckURI  = ( 28 << 8 | AUDIO_SVID ),
		PCS_ForwardToPeerURI = (9 << 8 | IMTRANS_SVID),
		PCS_ForwardToUserURI = (2001 << 8 | IMTRANS_SVID),
		PCS_ForwardToUserAckURI = (2003 << 8 | IMTRANS_SVID),
		PChangeInfoURI  = ( 31 << 8 | AUDIO_SVID ),
		PChangeInfoAckURI  = ( 32 << 8 | AUDIO_SVID ),
		PJoinChannel2URI = ( 19 << 8 | AUDIO_SVID ),
		PJoinChannelResURI  = ( 4 << 8 | AUDIO_SVID ),
		PLeaveChannelURI = ( 5 << 8 | AUDIO_SVID ),
		PLeaveChannelResURI = ( 6 << 8 | AUDIO_SVID ),
		PReGetMediaServerURI = ( 21 << 8 | AUDIO_SVID ),
		PReGetMediaServerResURI = ( 22 << 8 | AUDIO_SVID ),
		PRequestChannel2URI = ( 17 << 8 | AUDIO_SVID ),
		PRequestChannel2ResURI = ( 18 << 8 | AUDIO_SVID ),
		PStartCallURI = ( 23 << 8 | AUDIO_SVID ),
		PStartCallConfirmURI  = ( 33 << 8 | AUDIO_SVID ),
		PStartCallConfirmAckURI  = ( 34 << 8 | AUDIO_SVID ),
		PStartCallResURI = ( 25 << 8 | AUDIO_SVID ),
		PStopCallURI = ( 24 << 8 | AUDIO_SVID ),
		PStopCallAckURI = ( 35 << 8 | AUDIO_SVID ),

		// Group Chat
		PJoinChannelStringURI = (43 << 8 | AUDIO_SVID),
		PJoinChannelStringResURI = (44 << 8 | AUDIO_SVID),

		PQueryOnlineUsersReqURI     = ( 15 << 8 | AUDIO_SVID ),
		PQueryOnlineUsersResURI     = ( 16 << 8 | AUDIO_SVID ),

		PKickUserFromChannelURI = ( 23 << 8 | AUDIO_SVID ),
		PKickUserFromChannelResURI = ( 24 << 8 | AUDIO_SVID ),
		
		IMDB_SVID = 29,
		PCS_GetAppUserInfoByUidURI = (2009 << 8 | IMDB_SVID),
		PCS_GetAppUserInfoByUidResURI = (2010 << 8 | IMDB_SVID),
		PCS_GetUidByAppUserNameURI = (2011 << 8 | IMDB_SVID),
		PCS_GetUidByAppUserNameResURI = (2012 << 8 | IMDB_SVID),

		IMCHAT2D_SVID = 32,	//IMCHAT

		PCS_ChatMsgRes2URI = (2 << 8 | IMCHAT2D_SVID),
		PCS_ChatMsg2URI = (3 << 8 | IMCHAT2D_SVID),
		PCS_MultiRouteChatMsgURI = (13 << 8 | IMCHAT2D_SVID),
		PCS_MultiRouteChatMsgResURI = (14 << 8 | IMCHAT2D_SVID),
		PSS_MultiRouteChatMsgURI = (15 << 8 | IMCHAT2D_SVID),
		PSS_MultiRouteChatMsgResURI = (16 << 8 | IMCHAT2D_SVID),
		PCS_OnMessageArriveServerURI = (21 << 8 | IMCHAT2D_SVID),

		IMLINK_SUBSCRIBE = 130,

		PCS_SubcribeURI = (1 << 8 | IMLINK_SUBSCRIBE),

		IMOFFLINEMSGV2_SVID = 180,

		PCS_OfflineMsgUploadForceV2URI = (0 << 8 | IMOFFLINEMSGV2_SVID),
		PSC_OfflineMsgUploadResV2URI = (1 << 8 | IMOFFLINEMSGV2_SVID),
		PCS_OfflineMsgRequestNormalURI = (5 << 8 | IMOFFLINEMSGV2_SVID),
		PCS_OfflineMsgRequestNormalResURI = (6 << 8 | IMOFFLINEMSGV2_SVID),
		PCS_OfflineMsgNormalACKURI = (7 << 8 | IMOFFLINEMSGV2_SVID),

		IMGROUPCHAT_SVID = 131,
	//	PCS_ApplyCreateGroupChatURI = ((1 << 8) | IMGROUPCHAT_SVID);
	//	PCS_ApplyCreateGroupChatURI = ((1 << 8) | IMGROUPCHAT_SVID),
	//	PCS_ApplyCreateGroupChatResURI = ((2 << 8) | IMGROUPCHAT_SVID),
	//	PCS_ApplyInviteGroupChatURI = ((3 << 8) | IMGROUPCHAT_SVID),
	//	PCS_ApplyInviteGroupChatResURI = ((4 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatUserListURI = ((6 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatUserListResURI = ((7 << 8) | IMGROUPCHAT_SVID),
	//	PCS_LeaveGroupChatToServerURI = ((8 << 8) | IMGROUPCHAT_SVID),
		PCS_LeaveGroupChatURI = ((10 << 8) | IMGROUPCHAT_SVID),
	//	PCS_GroupChatMessageURI = ((11 << 8) | IMGROUPCHAT_SVID),
	//	PCS_GroupChatMessageMemberResURI = ((12 << 8) | IMGROUPCHAT_SVID),
		PCS_GroupChatTransmitMsgURI = ((36 << 8) | IMGROUPCHAT_SVID),
		PCS_GroupChatTransmitMsgResURI = ((37 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatOnlineURI = ((40 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatOnlineResURI = ((41 << 8) | IMGROUPCHAT_SVID),
		PCS_GroupChatPushDataURI = ((42 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatStatusURI = ((43 << 8) | IMGROUPCHAT_SVID),
		PCS_GroupChatStatusURI = ((44 << 8) | IMGROUPCHAT_SVID),
		PCS_BindGroupChatURI = ((45 << 8) | IMGROUPCHAT_SVID),
		PCS_BindGroupChatResURI = ((46 << 8) | IMGROUPCHAT_SVID),
		
	//	PCS_JoinGroupChatURI = ((27 << 8) | IMGROUPCHAT_SVID),
	//	PCS_JoinGroupChatResURI = ((28 << 8) | IMGROUPCHAT_SVID),
		
		
		PCS_UpdateGroupInfoURI =  (2001 << 8 | IMGROUPCHAT_SVID),
		PCS_UpdateGroupInfoResURI =  (2002 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupInfoURI =  (2003 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupInfoResURI = (2004 << 8 | IMGROUPCHAT_SVID),
	//	PCS_GetUserGroupURI =  (2005 << 8 | IMGROUPCHAT_SVID),
	//	PCS_GetUserGroupResURI =  (2006 << 8 | IMGROUPCHAT_SVID),
	//	PCS_GetUserGroupByNameURI =  (2007 << 8 | IMGROUPCHAT_SVID),
	//	PCS_GetUserGroupByNameResURI =  (2008 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupChatUserNameListURI = (2009 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupChatUserNameListResURI = (2010 << 8 | IMGROUPCHAT_SVID),
		
		PCS_AppApplyCreateGroupChatURI = (2011 << 8 | IMGROUPCHAT_SVID),
		PCS_AppApplyCreateGroupChatResURI = (2012 << 8 | IMGROUPCHAT_SVID),
		PCS_AppLeaveGroupChatURI = (2013 << 8 | IMGROUPCHAT_SVID),
		PCS_AppJoinGroupChatURI = (2014 << 8 | IMGROUPCHAT_SVID),
		PCS_AppJoinGroupChatResURI = (2015 << 8 | IMGROUPCHAT_SVID),
		PCS_AppApplyInviteGroupChatURI = (2016 << 8 | IMGROUPCHAT_SVID),
		PCS_AppApplyInviteGroupChatResURI = (2017 << 8 | IMGROUPCHAT_SVID),
		PCS_GetUserGroupByNameV2URI =  (2019 << 8 | IMGROUPCHAT_SVID),
		PCS_GetUserGroupByNameResV2URI =  (2020 << 8 | IMGROUPCHAT_SVID),
		
		PCS_GroupChatMessageFromUserURI =  (2021 << 8 | IMGROUPCHAT_SVID),
		PCS_GroupChatMessageFromUserResURI =  (2022 << 8 | IMGROUPCHAT_SVID),
		PSS_GroupChatMessageFromServerURI =  (2023 << 8 | IMGROUPCHAT_SVID),
		PCS_UpdateGroupChatMessageLastTimeURI =  (2024 << 8 | IMGROUPCHAT_SVID),
		PCS_UpdateGroupChatMessageLastTimeResURI =  (2025 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupChatOfflineMessageURI =  (2026 << 8 | IMGROUPCHAT_SVID),
		PCS_GetGroupChatOfflineMessageResURI =  (2027 << 8 | IMGROUPCHAT_SVID),
		PCS_GroupChatMessageAckURI =  (2028 << 8 | IMGROUPCHAT_SVID),
		PCS_GetMissedGroupChatMessageURI =  (2029 << 8 | IMGROUPCHAT_SVID),
		PCS_GetMissedGroupChatMessageResURI =  (2030 << 8 | IMGROUPCHAT_SVID),
		PCS_AppKickUserFromGroupChatURI =  (2031 << 8 | IMGROUPCHAT_SVID),
		PCS_AppKickUserFromGroupChatResURI =  (2032 << 8 | IMGROUPCHAT_SVID),
		PCS_AppCheckGroupOwnerURI =  (2033 << 8 | IMGROUPCHAT_SVID),
		PCS_AppCheckGroupOwnerResURI =  (2034 << 8 | IMGROUPCHAT_SVID),
		PSS_UpdateGroupInfoNotifyURI =  (2035 << 8 | IMGROUPCHAT_SVID),
		PCS_NotifiAppKickUserFromGroupChatURI =  (2036 << 8 | IMGROUPCHAT_SVID),
		PUpdateGroupRemarkURI = (2037 << 8 | IMGROUPCHAT_SVID),
		PUpdateGroupRemarkResURI = (2038 << 8 | IMGROUPCHAT_SVID),
		PUpdateGroupFlagURI = (2039 << 8 | IMGROUPCHAT_SVID),
		PUpdateGroupFlagResURI = (2040 << 8 | IMGROUPCHAT_SVID),
		PNotifyGroupUserInfoURI = (2041 << 8 | IMGROUPCHAT_SVID),
		PCS_BatchQueryGroupChatMessageLastTimeURI = ( 2042 << 8 | IMGROUPCHAT_SVID ),
		PCS_BatchQueryGroupChatMessageLastTimeResURI = ( 2043 << 8 | IMGROUPCHAT_SVID ),
	    
		PCS_GetGroupChatUserListRes2URI =  ((17 << 8) | IMGROUPCHAT_SVID),
		PCS_LeaveGroupChatToServer2URI =   ((18 << 8) | IMGROUPCHAT_SVID),
		PCS_LeaveGroupChat2URI =           ((19 << 8) | IMGROUPCHAT_SVID),
		PCS_GroupChatMessageMemberRes2URI = ((21 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupChatUserList2URI =     ((26 << 8) | IMGROUPCHAT_SVID),
		PCS_UserJoinMediaGroupURI =        ((60 << 8) | IMGROUPCHAT_SVID),
		PCS_UserJoinMediaGroupResURI =     ((61 << 8) | IMGROUPCHAT_SVID),
		PCS_UserLeaveMediaGroupURI =       ((62 << 8) | IMGROUPCHAT_SVID),
		PCS_UserLeaveMediaGroupResURI =    ((63 << 8) | IMGROUPCHAT_SVID),
		PStartMediaGroupCallURI =          ((64 << 8) | IMGROUPCHAT_SVID),
		PStartMediaGroupCallResURI =       ((65 << 8) | IMGROUPCHAT_SVID),
		PCS_InviteMediaGroupURI =          ((67 << 8) | IMGROUPCHAT_SVID),
		PCS_ApplyInviteMediaGroupAckURI =  ((68 << 8) | IMGROUPCHAT_SVID),
		PCS_ApplyInviteMediaGroupResURI =  ((69 << 8) | IMGROUPCHAT_SVID),
		PCS_ApplyInviteMediaGroupURI =     ((70 << 8) | IMGROUPCHAT_SVID),
		PCS_GetMediaGroupInfoURI =         ((71 << 8) | IMGROUPCHAT_SVID),
		PCS_GetMediaGroupInfoResURI =      ((72 << 8) | IMGROUPCHAT_SVID),
		PCS_InviteMediaGroupResURI =       ((74 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupStatusURI =            ((75 << 8) | IMGROUPCHAT_SVID),
		PCS_GetGroupStatusResURI =         ((76 << 8) | IMGROUPCHAT_SVID),
		PCS_MediaGroupPushDataURI =        ((80 << 8) | IMGROUPCHAT_SVID),
		PCS_SubscribeMediaGroupURI =       ((84 << 8) | IMGROUPCHAT_SVID),

		/* group store protocol */
		PUpdateGroupExtensionURI		=  ((2052 << 8) | IMGROUPCHAT_SVID),
		PUpdateGroupExtensionResURI		=  ((2053 << 8) | IMGROUPCHAT_SVID),
		PGetGroupExtensionURI	 		=  ((2054 << 8) | IMGROUPCHAT_SVID),
		PGetGroupExtensionResURI 		=  ((2055 << 8) | IMGROUPCHAT_SVID),
		PNotificationGroupExtensionURI	=  ((2056 << 8) | IMGROUPCHAT_SVID),
		
	};

	enum LoginResCode{
		LRC_SUCCESS = _RES_SUCCESS,
		LRC_INVALID_APPSECRET = 526,
		LRC_INVALID_COOKIE = 527,
		LRC_USER_AUTH_FAIL = 528,
		LRC_APP_BLACKLIST = 530 ,
		LRC_USER_BLACKLIST = 531,
	};

	enum InvalidProtocolData
	{
		MAX_PROTO_ELEMENT_COUNT = 512
	};

	static bool isValidLoginResCode(int code) 
	{
		bool rt = false;
		switch(code) {
		case LRC_SUCCESS:
		case LRC_INVALID_APPSECRET:
		case LRC_INVALID_COOKIE:
		case LRC_USER_AUTH_FAIL:
		case LRC_APP_BLACKLIST:
		case LRC_USER_BLACKLIST:
			rt = true;
			break;
		default:
			rt = false;
			break;
		}
		return rt;
	}

	static int loginResCodeToErrorCode(LoginResCode code) 
	{
		int rt = YYError::UNKNOWN;
		switch(code) {
		case LRC_INVALID_APPSECRET:
			rt = YYError::INVALID_APPSECRET;
			break;
		case LRC_INVALID_COOKIE:
			rt = YYError::INVALID_COOKIE;
			break;
		case LRC_USER_AUTH_FAIL:
			rt = YYError::USER_AUTH_FAIL;
			break;
		case LRC_APP_BLACKLIST:
			rt = YYError::APP_BLACKLIST;
			break;
		case LRC_USER_BLACKLIST:
			rt = YYError::USER_BLACKLIST;
			break;
		default:
			rt = YYError::UNKNOWN;
			break;
		};

		return rt;
	}
	
	static bool protoToByteBuffer(std::vector<unsigned char>& out, jint uri, const Marshallable& msg) {
		int size = msg.size();
		out.resize(size + HEAD_LEN);
		int pos = 0;
		pos += putValue_le(&out[pos], (jint)(size + HEAD_LEN));
		pos += putValue_le(&out[pos], uri);
		pos += putValue_le(&out[pos], (jshort)_RES_SUCCESS);
		
		size = out.size() - pos;
		if(msg.size() == 0) {
			if(msg.marshall(NULL, size) != size)
				return false;
		} else {
			if(msg.marshall(&out[pos], size) != size)
				return false;
		}
		return true;
	}

	/* for base type, such as char, short, int, long etc. */
	template <typename T>
	static int calcMarshallSize(const std::vector<T>& arr) 
	{
		return 4 + arr.size() * sizeof(T);
	}

	/* for Marshallable class */
	template <typename T>
	static int calcMarshallSize(const std::vector<T>& arr, T) 
	{
		int size = 0;
		int len = arr.size();
		for (int i = 0; i < len; i++)
			size += arr[i].size();
		return 4 + size;
	}

	/* for std::string */
	static int calcMarshallSize(const std::vector<std::string>& arr) 
	{
		int size = 0;
		int len = arr.size();
		for (int i = 0; i < len; i++)
			size += ProtoHelper::ShortSize::calcMarshallSize(arr[i]);
		return 4 + size;
	}

	template <typename T>
	static int calcMarshallSize(const std::map<T, std::string>& arr) 
	{
		int size = 0;
		int len = arr.size();

		std::map<T, std::string>::const_iterator iter;
		for (iter = arr.begin(); iter != arr.end(); ++iter) {
			size += sizeof(T);
			size += ProtoHelper::calcMarshallSize(iter->second);
		}
			
		return 4 + size;
	}

	template <typename K, typename V>
	static int calcMarshallSize(const std::map<K, V>& arr, V) 
	{
		int size = 0;
		int len = arr.size();

		std::map<K, V>::const_iterator iter;
		for (iter = arr.begin(); iter != arr.end(); ++iter) {
			size += sizeof(K);
			size += (iter->second).size();
		}
			
		return 4 + size;
	}

	//k, v are base type. such as char, short, int, long, long long etc.
	template <typename K, typename V>
	static int calcMarshallSize(const std::map<K, V>& arr) 
	{
		int size = 0;
		int len = arr.size();

		size += len * (sizeof(K) + sizeof(V));
			
		return 4 + size;
	}

	static int calcMarshallSize(const std::string& s) 
	{
		return 2 + s.size();
	}

	template <typename T>
	static int marshall(jbyte* out, const std::vector<T>& v) 
	{
		const int size = v.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);
		#if 0
		for (std::vector<T>::const_iterator iter = v.begin();
			iter != v.end(); ++iter)
		{
			pos += putValue_le(out + pos, (T)(*iter));
		}
		#else
		for (uint32_t i = 0; i < v.size(); i++) {
			pos += putValue_le(out + pos, v[i]);
		}
		#endif
		return pos;
	}

	template <typename T>
	static int marshall(jbyte* out, const std::vector<T>& v, T) 
	{
		const int size = v.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);
		for (std::vector<T>::const_iterator iter = v.begin();
			iter != v.end(); ++iter)
		{
			pos += ((T)(* iter)).marshall(out + pos, 0);
		}
		return pos;
	}

	static int marshall(jbyte* out, const std::vector<std::string>& v) 
	{
		const int size = v.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);
		for (std::vector<std::string>::const_iterator iter = v.begin();
			iter != v.end(); ++iter)
		{
			pos += ProtoHelper::ShortSize::marshall(out + pos, (*iter));
		}
		return pos;
	}

	static int marshall(jbyte* out, const std::string& s) 
	{	
		const jshort size = s.length();
		int pos = 0;
		pos += putValue_le(&out[pos], size);
		if(size > 0) {
			memcpy(&out[pos], &s[0], size);
			pos += size;
		}
		return pos;
	}

	template <typename K, typename V>
	static int marshall(jbyte *out, const std::map<K, V>& data)
	{
		const int size = data.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);

		std::map<K, V>::const_iterator iter;
		for (iter = data.begin(); iter != data.end(); ++ iter) {
			pos += putValue_le(&out[pos], iter->first);
			pos += putValue_le(&out[pos], iter->second);
		}
		return pos;
	}

	template <typename K, typename V>
	static int marshall(jbyte *out, const std::map<K, V>& data, V)
	{
		const int size = data.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);

		std::map<K, V>::const_iterator iter;
		for (iter = data.begin(); iter != data.end(); ++ iter) {
			pos += putValue_le(&out[pos], iter->first);
			(iter->second).marshall(out + pos, 0);
		}
		return pos;
	}

	template <typename T>
	static int marshall(jbyte *out, const std::map<T, std::string>& data)
	{
		const int size = data.size();
		int pos = 0;
		pos += putValue_le(&out[pos], size);

		std::map<T, std::string>::const_iterator iter;
		for (iter = data.begin(); iter != data.end(); ++ iter) {
			pos += putValue_le(&out[pos], iter->first);
			pos += ProtoHelper::marshall(out + pos, iter->second);
		}
		return pos;
	}


#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
	//for string.
	template <typename T>
	static int unmarshall(std::map<T, std::string>& out, const jbyte* buf, int bufSize) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, buf + pos);
		size = MIN(size, bufSize - pos);
		out.clear();

		T key;
		std::string value;
		for (int i = 0; i < size; ++i)
		{
			pos += getValue_le(&key, buf + pos);
			pos += ProtoHelper::unmarshall(value, buf + pos, bufSize - pos);

			out.insert(std::map<T, std::string>::value_type(key, value)); 
		}
		return pos;
	}

	//for base type.
	template <typename K, typename V>
	static int unmarshall(std::map<K, V>& out, const jbyte* buf, int bufSize) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, buf + pos);
		size = MIN(size, bufSize - pos);
		out.clear();

		K key;
		V value;
		for (int i = 0; i < size; ++i)
		{
			pos += getValue_le(&key, buf + pos);
			pos += getValue_le(&value, buf + pos);
			out.insert(std::map<K, V>::value_type(key, value)); 
		}
		return pos;
	}

	//for marshal class.
	template <typename K, typename V>
	static int unmarshall(std::map<K, V>& out, const jbyte* buf, int bufSize, V) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, buf + pos);
		size = MIN(size, bufSize - pos);
		out.clear();

		K key;
		V value;
		for (int i = 0; i < size; ++i)
		{
			pos += getValue_le(&key, buf + pos);
			value.unmarshall(buf + pos, bufSize - pos);
			pos += value.size();

			out.insert(std::map<K, V>::value_type(key, value)); 
		}
		return pos;
	}

	static int unmarshall(std::string& out, const jbyte* buf, int bufSize) 
	{
		int pos = 0;
		jshort size;
		pos += getValue_le(&size, &buf[pos]);
		size = MIN(size, bufSize-pos);
		if(size > 0) {
			out.assign((const char*)&buf[pos], size);
			pos += size;
		}else {
			out = "";
		}
		return pos;
	}

	template <typename T>
	static int unmarshall(std::vector<T>& out, const jbyte* buf, int bufSize) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, &buf[pos]);
		size = MIN(size, bufSize-pos);
		out.clear();
		out.reserve(size);
		T data;
		for (int i = 0; i < size; ++i)
		{
			pos += getValue_le(&data, buf+pos);
			out.push_back(data);
		}
		return pos;
	}

	template <typename T>
	static int unmarshall(std::vector<T>& out, const jbyte* buf, int bufSize, T) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, &buf[pos]);
		size = MIN(size, bufSize-pos);
		out.clear();
		out.reserve(size);
		T data;
		for (int i = 0; i < size; ++i)
		{
			data.unmarshall(buf + pos, bufSize - pos);
			pos += data.size();
			out.push_back(data);
		}
		return pos;
	}

	static int unmarshall(std::vector<std::string>& out, const jbyte* buf, int bufSize) 
	{
		int pos = 0;
		int size;
		pos += getValue_le(&size, &buf[pos]);
		size = MIN(size, bufSize-pos);
		out.clear();
		out.reserve(size);
		std::string data;
		for (int i = 0; i < size; ++i)
		{
			pos += ProtoHelper::ShortSize::unmarshall(data, buf + pos, bufSize - pos);
			out.push_back(data);
		}
		return pos;
	}

	//修复，如果跳过header，那data的长度也需要减少10byte，避免做就协议兼容时存在bug！
	static const jbyte* skipHeader(const jbyte* data, int & dataSize) 
	{
		dataSize -= HEAD_LEN;
		return data + HEAD_LEN;
	}

	class ShortSize
	{
	public:
		template <typename T>
		static inline int calcMarshallSize(const std::vector<T>& arr) 
		{
			return 2 + arr.size() * sizeof(T);
		}

		static inline int calcMarshallSize(const std::string& s) 
		{
			return 2 + s.size();
		}

		static inline int marshall(jbyte* out, const std::string& s) 
		{
			return ProtoHelper::marshall(out, s);
		}

		template <typename T>
		static int marshall(jbyte* out, const std::vector<T>& v) {
			const jshort size = v.size() * sizeof(T);
			int pos = 0;
			pos += putValue_le(&out[pos], size);
			if(size > 0) {
				memcpy(&out[pos], &v[0], size);
				pos += size;
			}
			return pos;
		}

		template <typename T>
		static int unmarshall(std::vector<T>& out, const jbyte* buf, int bufSize) 
		{
			int pos = 0;
			jshort size;
			pos += getValue_le(&size, &buf[pos]);
			size = MIN(size, bufSize-pos);
			if(size > 0) {
				out.resize((size+sizeof(T)-1)/sizeof(T));
				memcpy(&out[0], &buf[pos], size);
				pos += size;
			}else {
				out.clear();
			}
			return pos;
		}

		static inline int unmarshall(std::string& out, const jbyte* buf, int bufSize) 
		{
			return ProtoHelper::unmarshall(out, buf, bufSize);
		}
	};
};

#endif //__ProtoHelper_H