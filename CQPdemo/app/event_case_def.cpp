#pragma once
#include "event_case.h"
using std::vector;

namespace event_case
{
namespace evt
{
const case_pool::types_t types{
{ "古堡激战纪念箱", 100 },
{ "炼狱小镇纪念箱", 10 },
{ "死亡游乐园纪念箱", 10 },
{ "核子危机纪念箱", 5 },
{ "荒漠迷城纪念箱", 10 },
{ "炙热沙城II纪念箱", 7 },
{ "列车停放站纪念箱", 5 },
};

const case_pool::levels_t levels{
{ "消费级", 0.7647 },
{ "工业级", 0.1808 },
{ "军规级", 0.0421 },
{ "受限级", 0.0098 },
{ "保密级", 0.0021 },
{ "隐秘级", 0.0005 },
};

const vector<case_detail> cases{
{ 0, 0, "P90 | 暴风呼啸", 4 },
{ 0, 0, "SCAR-20 | 暴风呼啸", 3 },
{ 0, 0, "双持贝瑞塔 | 蔷薇", 3 },
{ 0, 0, "MAC-10 | 紫青之色", 11 },
{ 0, 0, "UMP-45 | 紫青之色", 11 },
{ 0, 1, "MAG-7 | 银质", 29 },
{ 0, 1, "新星 | 绿苹果", 5 },
{ 0, 1, "截短霰弹枪 | 外表生锈", 4 },
{ 0, 1, "USP 消音版 | 宝蓝之色", 142 },
{ 0, 2, "P2000 | 坚固链甲", 29 },
{ 0, 2, "MP9 | 黑暗时代", 40 },
{ 0, 3, "CZ75 自动手枪 | 复古圣杯", 499 },
{ 0, 3, "沙漠之鹰 | 手上加农炮", 863 },
{ 0, 4, "M4A1 消音型 | 骑士", 3860 },
{ 0, 5, "AWP | 巨龙传说", 188888 },
{ 1, 0, "UMP-45 | 泥地杀手", 1 },
{ 1, 0, "MP5-SD | 越野", 2 },
{ 1, 0, "MP9 | 滑移", 2 },
{ 1, 0, "AUG | 扫频仪", 3 },
{ 1, 0, "MAG-7 | 外表生锈", 1 },
{ 1, 1, "PP-野牛 | 红苹果", 3 },
{ 1, 1, "MAC-10 | 小牛皮", 11 },
{ 1, 1, "R8 左轮手枪 | 氮化处理", 6 },
{ 1, 1, "格洛克 18 型 | 远光灯", 49 },
{ 1, 2, "SSG 08 | 手刹", 50 },
{ 1, 2, "M4A4 | 变频器", 85 },
{ 1, 2, "USP 消音版 | 引擎故障灯", 80 },
{ 1, 2, "截短霰弹枪 | 刹车灯", 18 },
{ 1, 3, "P250 | 葡萄酒", 150 },
{ 1, 3, "MP7 | 渐变之色", 112 },
{ 1, 3, "AK-47 | 安全网", 349 },
{ 1, 4, "双持贝瑞塔 | 双涡轮", 501 },
{ 1, 4, "SG 553 | 意式拉力", 1782 },
{ 2, 0, "M249 | 对比涂装", 1 },
{ 2, 0, "MAG-7 | 暴风呼啸", 1 },
{ 2, 0, "MP9 | 暴风呼啸", 2 },
{ 2, 0, "截短霰弹枪 | 贤者涂装", 1 },
{ 2, 0, "UMP-45 | 枯焦之色", 1 },
{ 2, 1, "MP7 | 硝烟", 5 },
{ 2, 1, "沙漠之鹰 | 都市 DDPAT", 31 },
{ 2, 1, "格洛克 18 型 | 噩梦之夜", 18 },
{ 2, 1, "P2000 | 工业牧草", 4 },
{ 2, 2, "XM1014 | 蔚蓝多变迷彩", 5 },
{ 2, 2, "SSG 08 | 迂回路线", 30 },
{ 2, 2, "CZ75 自动手枪 | 氮化处理", 36 },
{ 2, 3, "AWP | 粉红 DDPAT", 540 },
{ 2, 3, "USP 消音版 | 公路杀手", 332 },
{ 2, 4, "M4A1 消音型 | 神来之作", 1325 },
{ 3, 1, "内格夫 | 核子废渣", 6 },
{ 3, 1, "P250 | 核子污染", 9 },
{ 3, 1, "AUG | 辐射危机", 32 },
{ 3, 1, "PP-野牛 | 化工之绿", 6 },
{ 3, 1, "FN57 | 热火朝天", 8 },
{ 3, 1, "SG 553 | 辐射警告", 36 },
{ 3, 2, "格洛克 18 型 | 核子反应", 151 },
{ 3, 2, "MP9 | 落日", 55 },
{ 3, 2, "XM1014 | 碾骨机", 57 },
{ 3, 2, "MAC-10 | 核子花园", 54 },
{ 3, 2, "Tec-9 | 核子剧毒", 36 },
{ 3, 3, "法玛斯 | 冥界之憎", 117 },
{ 3, 3, "加利尔 AR | 地狱看门犬", 223 },
{ 4, 0, "P250 | 骸骨外罩", 3 },
{ 4, 0, "FN57 | 雇佣兵", 2 },
{ 4, 0, "AUG | 殖民侵略者", 5 },
{ 4, 0, "G3SG1 | 狩猎网格", 1 },
{ 4, 0, "P90 | 枯焦之色", 2 },
{ 4, 0, "加利尔 AR | 隐蔽猎手", 3 },
{ 4, 1, "格洛克 18 型 | 地下水", 22 },
{ 4, 1, "MP7 | 橘皮涂装", 8 },
{ 4, 1, "SSG 08 | 热带风暴", 22 },
{ 4, 1, "内格夫 | 加州迷彩", 5 },
{ 4, 1, "SG 553 | 鳄鱼网格", 16 },
{ 4, 2, "MP9 | 赤红新星", 144 },
{ 4, 2, "UMP-45 | 炽烈之炎", 261 },
{ 4, 2, "MAC-10 | 渐变琥珀", 45 },
{ 4, 3, "MAG-7 | 威吓者", 71 },
{ 5, 0, "G3SG1 | 沙漠风暴", 2 },
{ 5, 0, "P250 | 沙丘之黄", 5 },
{ 5, 0, "SCAR-20 | 沙漠网格", 1 },
{ 5, 0, "P90 | 沙漠涂装", 2 },
{ 5, 0, "MP9 | 沙漠虚线", 2 },
{ 5, 0, "新星 | 捕食者", 1 },
{ 5, 1, "截短霰弹枪 | 蝮蛇迷彩", 2 },
{ 5, 1, "AK-47 | 狩猎网格", 86 },
{ 5, 1, "FN57 | 橘皮涂装", 11 },
{ 5, 1, "MAC-10 | 棕榈色", 12 },
{ 5, 1, "Tec-9 | 多变迷彩", 6 },
{ 5, 2, "PP-野牛 | 黄铜", 23 },
{ 5, 2, "M4A1 消音型 | 多变迷彩", 55 },
{ 5, 2, "SG 553 | 大马士革钢", 75 },
{ 5, 3, "P2000 | 渐变琥珀", 47 },
{ 5, 3, "R8 左轮手枪 | 渐变琥珀", 162 },
{ 6, 0, "UMP-45 | 都市 DDPAT", 5 },
{ 6, 0, "双持贝瑞塔 | 殖民侵略者", 2 },
{ 6, 0, "G3SG1 | 极地迷彩", 5 },
{ 6, 0, "FN57 | 暮色森林", 4 },
{ 6, 0, "新星 | 极地网格", 2 },
{ 6, 0, "PP-野牛 | 都市虚线", 2 },
{ 6, 1, "MAC-10 | 红苹果", 16 },
{ 6, 1, "M4A4 | 都市 DDPAT", 58 },
{ 6, 1, "MAG-7 | 金属 DDPAT", 8 },
{ 6, 1, "P250 | 金属 DDPAT", 13 },
{ 6, 1, "SCAR-20 | 碳素纤维", 9 },
{ 6, 1, "P90 | 白蜡木", 6 },
{ 6, 2, "沙漠之鹰 | 都市瓦砾", 36 },
{ 6, 2, "截短霰弹枪 | 渐变琥珀", 6 },
{ 6, 3, "Tec-9 | 晶红石英", 29 },
};
}

namespace drop
{
const case_pool::types_t types{
{ "无畏收藏品", 0 },
{ "仓库突击收藏品", 0 },
{ "雨林遗迹收藏品", 0 },
{ "金库危机收藏品", 0 },
{ "解体厂收藏品", 0 },
{ "神魔收藏品", 0 },
{ "意大利小镇收藏品", 0 },
{ "湖畔激战收藏品", 0 },
{ "佣兵训练营收藏品", 0 },
{ "办公室收藏品", 0 },
{ "旭日收藏品", 0 },
{ "安全处所收藏品", 0 },
{ "殒命大厦收藏品", 0 },
};

const case_pool::levels_t levels{
{ "消费级", 0.7646 },
{ "工业级", 0.1808 },
{ "军规级", 0.0421 },
{ "受限级", 0.0098 },
{ "保密级", 0.0021 },
{ "隐秘级", 0.0005 },
{ "违禁级", 0.0001 },
};

const vector<case_detail> cases{
{ 0, 0, "M249 | 丛林 DDPAT", 5 },
{ 0, 0, "Tec-9 | 狂哮飓风", 4 },
{ 0, 0, "MP9 | 干旱季节", 5 },
{ 0, 0, "FN57 | 电镀青铜", 11 },
{ 0, 0, "XM1014 | 狂野丛林", 6 },
{ 0, 0, "MP7 | 地下水", 9 },
{ 0, 1, "SSG 08 | 玛雅之梦", 19 },
{ 0, 1, "格洛克 18 型 | 沙丘之黄", 27 },
{ 0, 1, "内格夫 | 棕榈色", 26 },
{ 0, 1, "P250 | 多面体", 17 },
{ 0, 1, "截短霰弹枪 | 马赛克", 19 },
{ 0, 2, "MAG-7 | 危机色调", 41 },
{ 0, 2, "PP-野牛 | 外表生锈", 31 },
{ 0, 2, "AUG | 深蓝电镀处理", 183 },
{ 0, 3, "SCAR-20 | 翡翠色调", 182 },
{ 0, 3, "法玛斯 | 喷焰者", 186 },
{ 0, 6, "M4A4 | 咆哮", 18888 },
{ 1, 0, "SG 553 | 狂哮飓风", 27 },
{ 1, 0, "UMP-45 | 色如焦糖", 26 },
{ 1, 1, "FN57 | 红苹果", 120 },
{ 1, 2, "内格夫 | 深蓝电镀处理", 447 },
{ 1, 2, "AUG | 赤红新星", 548 },
{ 1, 3, "MP9 | 威吓者", 443 },
{ 1, 3, "格洛克 18 型 | 渐变之色", 2791 },
{ 2, 0, "FN57 | 狂野丛林", 7 },
{ 2, 0, "新星 | 森林之叶", 6 },
{ 2, 0, "SSG 08 | 青苔虚线", 8 },
{ 2, 1, "M4A4 | 丛林虎", 29 },
{ 2, 1, "AK-47 | 丛林涂装", 39 },
{ 2, 2, "Tec-9 | 骨化之色", 10 },
{ 3, 0, "SSG 08 | 沙丘之黄", 12 },
{ 3, 0, "CZ75 自动手枪 | 绿色格纹", 4 },
{ 3, 0, "MP9 | 绿色格纹", 3 },
{ 3, 0, "G3SG1 | 雇佣兵", 3 },
{ 3, 0, "MP7 | 橄榄格纹", 2 },
{ 3, 1, "P2000 | 廉价皮革", 14 },
{ 3, 1, "P90 | 棕色皮革", 14 },
{ 3, 1, "MAC-10 | 通勤者皮革", 15 },
{ 3, 1, "SG 553 | 旅行者皮革", 18 },
{ 3, 2, "截短霰弹枪 | 至高皮革", 71 },
{ 3, 2, "USP 消音版 | 商业皮革", 74 },
{ 3, 2, "XM1014 | 红色皮革", 67 },
{ 3, 3, "沙漠之鹰 | 飞行员", 261 },
{ 3, 3, "AK-47 | 至高皮革", 229 },
{ 3, 4, "AK-47 | 酷炫涂鸦皮革", 615 },
{ 4, 0, "MAG-7 | 海鸟", 2 },
{ 4, 0, "CZ75 自动手枪 | 军队之辉", 2 },
{ 4, 0, "M249 | 冲击钻", 2 },
{ 4, 0, "SCAR-20 | 军队之辉", 2 },
{ 4, 1, "加利尔 AR | 都市瓦砾", 8 },
{ 4, 1, "USP 消音版 | 绿色伞兵", 9 },
{ 4, 1, "沙漠之鹰 | 噩梦之夜", 18 },
{ 4, 2, "FN57 | 氮化处理", 37 },
{ 4, 2, "MP7 | 逮捕者", 53 },
{ 4, 2, "P250 | 银装素裹", 62 },
{ 4, 2, "CZ75 自动手枪 | 翡翠色调", 58 },
{ 4, 2, "MAC-10 | 渐变之色", 100 },
{ 4, 3, "双持贝瑞塔 | 决斗家", 78 },
{ 4, 3, "SG 553 | 威吓者", 380 },
{ 4, 4, "格洛克 18 型 | 暮光星系", 109 },
{ 4, 4, "M4A1 消音型 | 赤红新星", 626 },
{ 5, 0, "新星 | 天秤之月", 4 },
{ 5, 0, "双持贝瑞塔 | 天秤之月", 3 },
{ 5, 0, "MP7 | 星点", 3 },
{ 5, 0, "AUG | 代达罗斯之殇", 3 },
{ 5, 1, "Tec-9 | 哈迪斯", 15 },
{ 5, 1, "P2000 | 寻路者", 15 },
{ 5, 1, "AWP | 狮子之日", 28 },
{ 5, 1, "M249 | 海滨预测者", 14 },
{ 5, 2, "UMP-45 | 弥诺陶洛斯迷宫", 119 },
{ 5, 2, "MP9 | 潘多拉魔盒", 116 },
{ 5, 3, "M4A1 消音型 | 伊卡洛斯殒落", 517 },
{ 5, 3, "G3SG1 | 柯罗诺斯", 428 },
{ 5, 4, "M4A4 | 波塞冬", 2708 },
{ 5, 5, "AWP | 美杜莎", 12888 },
{ 6, 0, "新星 | 沙丘之黄", 1 },
{ 6, 0, "PP-野牛 | 沙漠虚线", 1 },
{ 6, 0, "AUG | 雇佣兵", 1 },
{ 6, 0, "法玛斯 | 殖民侵略者", 1 },
{ 6, 0, "Tec-9 | 地下水", 1 },
{ 6, 1, "新星 | 红苹果", 1 },
{ 6, 1, "双持贝瑞塔 | 人工染色", 1 },
{ 6, 1, "P2000 | 坚毅大理石纹", 1 },
{ 6, 1, "UMP-45 | 硝烟", 1 },
{ 6, 1, "M4A1 消音型 | 北方森林", 2 },
{ 6, 1, "XM1014 | 加州迷彩", 1 },
{ 6, 2, "MP7 | 深蓝电镀处理", 2 },
{ 6, 2, "格洛克 18 型 | 红苹果", 3 },
{ 6, 2, "截短霰弹枪 | 逮捕者", 2 },
{ 6, 3, "AWP | 响尾蛇", 9 },
{ 7, 0, "G3SG1 | 丛林虚线", 1 },
{ 7, 0, "SG 553 | 浪花穿孔", 1 },
{ 7, 0, "加利尔 AR | 贤者涂装", 1 },
{ 7, 0, "AUG | 暴风呼啸", 1 },
{ 7, 0, "XM1014 | 蓝色云杉", 1 },
{ 7, 0, "P250 | 北方森林", 1 },
{ 7, 1, "XM1014 | 蓝钢", 1 },
{ 7, 1, "法玛斯 | 湖蓝涂装", 1 },
{ 7, 1, "PP-野牛 | 午夜行动", 1 },
{ 7, 1, "沙漠之鹰 | 泥地杀手", 1 },
{ 7, 1, "AWP | 狩猎网格", 3 },
{ 7, 2, "P90 | 摧枯拉朽", 1 },
{ 7, 2, "SG 553 | 深蓝电镀处理", 3 },
{ 7, 2, "USP 消音版 | 午夜行动", 4 },
{ 7, 3, "双持贝瑞塔 | 钴蓝石英", 2 },
{ 8, 0, "PP-野牛 | 森林之叶", 6 },
{ 8, 0, "MAC-10 | 狂哮飓风", 6 },
{ 8, 0, "XM1014 | 工业牧草", 6 },
{ 8, 1, "P2000 | 草原落叶", 75 },
{ 8, 2, "新星 | 火焰橙", 55 },
{ 8, 2, "XM1014 | 火焰橙", 58 },
{ 8, 2, "P250 | 现代猎手", 123 },
{ 8, 2, "新星 | 现代猎手", 55 },
{ 8, 2, "PP-野牛 | 现代猎手", 157 },
{ 8, 3, "M4A4 | 现代猎手", 196 },
{ 8, 4, "SCAR-20 | 溅射果酱", 157 },
{ 9, 0, "法玛斯 | 对比涂装", 12 },
{ 9, 1, "加利尔 AR | 冬之森林", 46 },
{ 9, 1, "G3SG1 | 极寒迷彩", 45 },
{ 9, 1, "M249 | 暴雪大理石纹", 45 },
{ 9, 2, "P2000 | 银质", 45 },
{ 9, 2, "MP7 | 银装素裹", 173 },
{ 10, 0, "P250 | 日式薄荷涂装", 3 },
{ 10, 0, "截短霰弹枪 | 竹影", 3 },
{ 10, 0, "Tec-9 | 竹林", 3 },
{ 10, 0, "G3SG1 | 日式橙色涂装", 3 },
{ 10, 0, "PP-野牛 | 墨竹", 3 },
{ 10, 1, "沙漠之鹰 | 午夜风暴", 27 },
{ 10, 1, "P250 | 日式深红涂装", 9 },
{ 10, 2, "Tec-9 | 梗怖分子", 83 },
{ 10, 2, "MAG-7 | 反梗精英", 75 },
{ 10, 2, "加利尔 AR | 水蓝阶地", 81 },
{ 10, 3, "M4A4 | 破晓", 261 },
{ 10, 3, "沙漠之鹰 | 日落风暴 壱", 258 },
{ 10, 3, "FN57 | 日式荧光涂装", 231 },
{ 10, 3, "沙漠之鹰 | 日落风暴 弐", 264 },
{ 10, 4, "AK-47 | 水栽竹", 1977 },
{ 10, 5, "AUG | 秋叶原之选", 7999 },
{ 11, 0, "MP7 | 陆军斥候", 1 },
{ 11, 0, "Tec-9 | 陆军网格", 1 },
{ 11, 0, "SSG 08 | 蓝色云杉", 1 },
{ 11, 0, "双持贝瑞塔 | 雇佣兵", 1 },
{ 11, 0, "SCAR-20 | 雇佣兵", 1 },
{ 11, 1, "USP 消音版 | 森林之叶", 2 },
{ 11, 1, "MP9 | 橘皮涂装", 1 },
{ 11, 1, "加利尔 AR | 多变迷彩", 1 },
{ 11, 1, "M249 | 鳄鱼网格", 1 },
{ 11, 1, "G3SG1 | 多变迷彩", 1 },
{ 11, 1, "AUG | 众枪之的", 1 },
{ 11, 2, "法玛斯 | 摧枯拉朽", 2 },
{ 11, 2, "FN57 | 银白石英", 2 },
{ 11, 2, "SSG 08 | 渐变强酸", 4 },
{ 11, 3, "M4A1 消音型 | 氮化处理", 11 },
{ 12, 0, "XM1014 | 都市穿孔", 12 },
{ 12, 0, "MAC-10 | 都市 DDPAT", 7 },
{ 12, 1, "PP-野牛 | 碳素纤维", 40 },
{ 12, 2, "P90 | 冰川网格", 25 },
{ 12, 2, "AK-47 | 黑色层压板", 152 },
{ 12, 3, "双持贝瑞塔 | 爆破能手", 11 },
};
}
}
