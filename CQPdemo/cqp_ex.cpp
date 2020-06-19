#include "cqp_ex.h"
#include "cqp.h"
#include "app/common/dbconn.h"

#include "app/private/qqid.h"

// ntohs, ntohl, ntohll
#include <WinSock2.h>

#include "cpp-base64/base64.h"

GroupMemberInfo::GroupMemberInfo(const char* base64_decoded)
{
	size_t offset = 0;
	int16_t len = 0;

	group = ntohll(*(uint64_t*)(&base64_decoded[offset]));
	offset += 8;

	qqid = ntohll(*(uint64_t*)(&base64_decoded[offset]));
	offset += 8;

	len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
	offset += 2;
	nick = simple_str(len, &base64_decoded[offset]);
	offset += len;

	len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
	offset += 2;
	card = simple_str(len, &base64_decoded[offset]);
	offset += len;

	gender = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	age = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
	offset += 2;
	area = simple_str(len, &base64_decoded[offset]);
	offset += len;

	joinTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	speakTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
	offset += 2;
	level = simple_str(len, &base64_decoded[offset]);
	offset += len;

	permission = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	dummy1 = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
	offset += 2;
	title = simple_str(len, &base64_decoded[offset]);
	offset += len;

	titleExpireTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;

	canModifyCard = ntohl(*(uint32_t*)(&base64_decoded[offset]));
	offset += 4;
}


std::map<int64_t, GroupMemberInfo> getGroupMemberList(int64_t group_id)
{
	std::map<int64_t, GroupMemberInfo> members;
	const char* grpbuf = CQ_getGroupMemberList(ac, group_id);

	if (grpbuf == NULL)
	{
		members.clear();
		return members;
	}

	std::string raw = base64_decode(std::string(grpbuf));
	int count = ntohl(*((uint32_t*)(raw.c_str())));
	const char* p = raw.c_str() + sizeof(uint32_t);

	members.clear();

	for (int i = 0; (i < count) && (p < raw.c_str() + raw.length()); ++i)
	{
		int member_size = ntohs(*((uint16_t*)(p)));
		p += sizeof(uint16_t);

		auto m = GroupMemberInfo(p);
		members[m.qqid] = m;
		p += member_size;
	}
	return members;
}


std::string getCardFromGroupInfoV2(const char* base64_decoded)
{
	/*
	size_t nick_offset = 8 + 8;
	size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
	size_t card_offset = nick_offset + 2 + nick_len;
	size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
	if (card_len == 0)
		return std::string(&base64_decoded[nick_offset+2], nick_len);
	else
		return std::string(&base64_decoded[card_offset+2], card_len);
		*/
	return GroupMemberInfo(base64_decoded).card;
}

int getPermissionFromGroupInfoV2(const char* base64_decoded)
{
	/*
	size_t nick_offset = 8 + 8;
	size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
	size_t card_offset = nick_offset + 2 + nick_len;
	size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
	size_t area_offset = card_offset + 2 + card_len + 4 + 4;
	size_t area_len = (base64_decoded[area_offset] << 8) + base64_decoded[area_offset + 1];
	size_t glvl_offset = area_offset + 2 + area_len + 4 + 4;
	size_t glvl_len = (base64_decoded[glvl_offset] << 8) + base64_decoded[glvl_offset + 1];
	size_t perm_offset = glvl_offset + 2 + glvl_len;
	return (base64_decoded[perm_offset + 0] << (8 * 3)) +
		   (base64_decoded[perm_offset + 1] << (8 * 2)) +
		   (base64_decoded[perm_offset + 2] << (8 * 1)) +
		   (base64_decoded[perm_offset + 3]);
		   */
	return GroupMemberInfo(base64_decoded).permission;
}

#include "cqp.h"
#include "app/data/group.h"
#include "cpp-base64/base64.h"
std::string getCard(int64_t group, int64_t qq)
{
	if (grp::groups.find(group) != grp::groups.end())
	{
		if (grp::groups[group].haveMember(qq))
			return grp::groups[group].members[qq].card.length() != 0 ?
			grp::groups[group].members[qq].card : grp::groups[group].members[qq].nick;
		else
			return CQ_At(qq);
	}
	else
	{
		std::string qqname;
		const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
		if (cqinfo && strlen(cqinfo) > 0)
		{
			std::string decoded = base64_decode(std::string(cqinfo));
			if (!decoded.empty())
			{
				qqname = getCardFromGroupInfoV2(decoded.c_str());
			}
		}
		if (qqname.empty()) qqname = CQ_At(qq);
		return qqname;
	}
}

bool isGroupManager(int64_t group, int64_t qq)
{
	if (qq == qqid_admin) return true;
	const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, TRUE);
	if (cqinfo && strlen(cqinfo) > 0)
	{
		std::string decoded = base64_decode(std::string(cqinfo));
		if (!decoded.empty())
		{
			return (getPermissionFromGroupInfoV2(decoded.c_str()) >= 2);
		}
	}
	return false;
}

bool isGroupOwner(int64_t group, int64_t qq)
{
	const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, TRUE);
	if (cqinfo && strlen(cqinfo) > 0)
	{
		std::string decoded = base64_decode(std::string(cqinfo));
		if (!decoded.empty())
		{
			return (getPermissionFromGroupInfoV2(decoded.c_str()) >= 3);
		}
	}
	return false;
}
