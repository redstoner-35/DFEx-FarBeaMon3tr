#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

//外部参考
extern EnumEditEntryDef DynamicPowerCfg[7];
extern const char EmptyStr[];

//PDO显示数值
const EnumEditEntryDef PDOValueEnum[8]=
	{
		{
		"不存在可用的PDO",
	  true,
		0,
		false,
		},
		{
		"PDO获取失败",
	  true,
		1,
		false,
		},
		{
		"5V",
	  false,
		2,
		false,
		},
		{
		"9V",
	  false,
		3,
		false,
		},	
		{
		"12V",
	  false,
		4,
		false,
		},		
		{
		"15V",
	  true,
		5,
		false,
		},	
		{
		"20V",
	  true,
		6,
		false,
		},	
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

//准备PDO List配置
void PreparePDOListCfg(void)
	{
	int i,index;
	bool PDOState;
	RecvPDOListDef PDOResult;
	//获取PDO状态
	if(!IP2366_GetRecvPDOList(&PDOResult))for(i=0;i<7;i++)
		{
		//PDO获取失败，显示未知的PDO
		if(i==0)
			{
			DynamicPowerCfg[0].IsChinese=true;
			DynamicPowerCfg[0].EnumValue=0;
			DynamicPowerCfg[0].IsPlaceHolder=false;
			DynamicPowerCfg[0].SelName=PDOValueEnum[1].SelName;
			}
		else
			{
			DynamicPowerCfg[0].IsChinese=false;
			DynamicPowerCfg[i].EnumValue=100;
			DynamicPowerCfg[i].IsPlaceHolder=true;
			DynamicPowerCfg[i].SelName=(char *)EmptyStr;			
			}
		}
	//获取成功，开始执行判断和PDO构建处理
	else 
		{
		index=0;
		//根据读回来的PDO List判断结果
		for(i=0;i<5;i++)
		  {
			//根据当前执行的index转换对应的PDO使能
			switch(i)
				{
				case 0:PDOState=PDOResult.PDO5VOK;break;
				case 1:PDOState=PDOResult.PDO9VOK;break;
				case 2:PDOState=PDOResult.PDO12VOK;break;
				case 3:PDOState=PDOResult.PDO15VOK;break;
				case 4:PDOState=PDOResult.PDO20VOK;break;
				}
			//当前PDO不存在，跳过循环
			if(!PDOState)continue;
			//当前PDO存在，开始搬运选项	
			DynamicPowerCfg[index].EnumValue=index;
			DynamicPowerCfg[index].IsPlaceHolder=false;
			DynamicPowerCfg[index].IsChinese=PDOValueEnum[i+2].IsChinese;
			DynamicPowerCfg[index].SelName=PDOValueEnum[i+2].SelName;	
			//index构建完毕，开始下一项
			index++;
			}
		//如果index等于0则说明没有合适的PDO，提示没有可用的PDO
		if(index==0)
			{
			DynamicPowerCfg[0].IsChinese=true;
			DynamicPowerCfg[0].EnumValue=0;
			DynamicPowerCfg[0].IsPlaceHolder=false;
			DynamicPowerCfg[0].SelName=PDOValueEnum[0].SelName;		
			index=1;
			}
		//将剩下的空间区域填充PlaceHolder
		for(i=index;i<7;i++)
			{
			DynamicPowerCfg[index].EnumValue=100;
			DynamicPowerCfg[index].IsPlaceHolder=true;
			DynamicPowerCfg[index].IsChinese=false;
			DynamicPowerCfg[index].SelName=(char *)EmptyStr;
			}
		}
	}

//初始化PDO显示列表	
int InitPDOQueryList(void)
	{
	//PDO列表固定从最开头的项目开始
	return 0;
	}	
	
//退出PDO显示列表
void ReturnPDOQueryList(int Input)
	{
	int i;
	for(i=0;i<7;i++)
		{
		DynamicPowerCfg[i].EnumValue=100;
		DynamicPowerCfg[i].IsPlaceHolder=true;
		DynamicPowerCfg[i].IsChinese=false;
		DynamicPowerCfg[i].SelName=(char *)EmptyStr;
		}	
	//返回到对应设置菜单
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}
	
const MenuConfigDef QueryPDOListMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	DynamicPowerCfg,
  &InitPDOQueryList,
  &ReturnPDOQueryList,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"Sink PDO列表查询",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	&PreparePDOListCfg,
	NULL
	};
