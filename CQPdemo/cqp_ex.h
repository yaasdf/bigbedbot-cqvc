#pragma once

#include <string>
#include <map>

#include "utils/simple_str.h"

enum enumCQBOOL
{
	FALSE,
	TRUE,
};

/*
Ⱥ��Ա��Ϣ
��**CQ_getGroupMemberInfoV2**���ص���Ϣ
ǰ8���ֽڣ���һ��Int64_t���ȣ�QQȺ�ţ�
������8���ֽڣ���һ��Int64_t���ȣ�QQ�ţ�
������2���ֽڣ���һ��short���ȣ��ǳƳ��ȣ�
�������ǳƳ��ȸ��ֽڣ��ǳ��ı���
������2���ֽڣ���һ��short���ȣ�Ⱥ��Ƭ���ȣ�
������Ⱥ��Ƭ���ȸ��ֽڣ�Ⱥ��Ƭ�ı���
������4���ֽڣ���һ��int���ȣ��Ա�0��1Ů��
������4���ֽڣ���һ��int���ȣ����䣬QQ�ﲻ��ֱ���޸����䣬�Գ�����Ϊ׼��
������2���ֽڣ���һ��short���ȣ��������ȣ�
�������������ȸ��ֽڣ������ı���
������4���ֽڣ���һ��int���ȣ���Ⱥʱ�����
������4���ֽڣ���һ��int���ȣ������ʱ�����
������2���ֽڣ���һ��short���ȣ�Ⱥ�ȼ����ȣ�
������Ⱥ�ȼ����ȸ��ֽڣ�Ⱥ�ȼ��ı���
������4���ֽڣ���һ��int���ȣ�����Ȩ�ޣ�1��Ա��2����Ա��3Ⱥ����
������4���ֽڣ���һ��int���ȣ�0����֪����ʲô�������ǲ�����¼��Ա��
������2���ֽڣ���һ��short���ȣ�ר��ͷ�γ��ȣ�
������ר��ͷ�γ��ȳ��ȸ��ֽڣ�ר��ͷ�γ����ı���
������4���ֽڣ���һ��int���ȣ�ר��ͷ�ι���ʱ�����
������4���ֽڣ���һ��int���ȣ������޸���Ƭ��1�����²�0�ǲ�����
*/

class GroupMemberInfo
{
public:
	int64_t group;
	int64_t qqid;
	simple_str nick;
	simple_str card;
	int32_t gender;
	int32_t age;
	simple_str area;
	int32_t joinTime;
	int32_t speakTime;
	simple_str level;
	int32_t permission;
	int32_t dummy1;
	simple_str title;
	int32_t titleExpireTime;
	int32_t canModifyCard;

	GroupMemberInfo() {}
	GroupMemberInfo(const char* base64_decoded);
};

std::map<int64_t, GroupMemberInfo> getGroupMemberList(int64_t grpid);

//card: 8+8+2+?+2+?
std::string getCardFromGroupInfoV2(const char* base64_decoded);
std::string getCard(int64_t group, int64_t qq);

// 1��Ա��2����Ա��3Ⱥ��
int getPermissionFromGroupInfoV2(const char* base64_decoded);

inline std::string CQ_At(int64_t qq)
{
	using namespace std::string_literals;
	return "[CQ:at,qq="s + std::to_string(qq) + "]"s;
}

bool isGroupManager(int64_t group, int64_t qq);
bool isGroupOwner(int64_t group, int64_t qq);

inline const std::string EMOJI_HORSE = "[CQ:emoji,id=128052]";
inline const std::string EMOJI_HAMMER = "[CQ:emoji,id=128296]";
inline const std::string EMOJI_DOWN = "[CQ:emoji,id=11015]";
inline const std::string EMOJI_NONE = "[CQ:emoji,id=127514]";
inline const std::string EMOJI_HORN = "[CQ:emoji,id=128227]";