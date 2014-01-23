#include <dos.h>
#include <stdio.>
#include <stdlib.h>
#include <conio.h>
#include <bios.h>

/*将按字表示的字符串转换成可显示的ASCII字符串格式*/
char *string( unsigned int in_data[], int start, int end )
{
static char ret_str[40];
int loop,loopl;
for ( loop = start,loopl = 0; loop <= end; loop++ )
{
ret_str[loopl++] = (char)(in_data[loop]/256);/*取字的高字节*/
ret_str[loopl++] = (char)(in_data[loop]%256);/*取字的低字节*/
}
ret_str[loopl] = ' \0';   /*字符串以0结束*/
return(ret_str);
}

void main( void )
{
unsigned int hd_info[256]; /*硬盘物理参数暂存区*/
unsigned int info_point;   /*信息缓冲区指针*/
unsigned int loop;         /*循环指针*/
unsigned int hd_number = 0;/*系统物理硬盘数*/
unsigned int hd_size;      /*硬盘容量*/
union REGS registers;
/*以下变量定义为CMOS SETUP中设置的逻辑格式化柱面数，磁头数，扇区数 */
unsigned int cmos_xyl[2], cmos_head[2], cmos_sec[2];

clrscr();
printf("\n***IDE(EIDE)硬盘物理及逻辑格式化参数检测***\n");
for ( loop = 0; loop < 2; loop++)
{
while (inportb(0x1f7) >= 0x80; /*等待硬盘控制器准备好*/
/* 硬盘端口1F7读（状态寄存器）定义：
   位0：=1 命令执行出错
     1: =1 收到索引（每当磁盘旋转一周，该位置1）
     2: =1 ECC检验错误（读取的数据被ECC算法改正）
     3: =1 服务请求（在读写时，扇区缓冲请求数据输入输出服务）
     4: =1 寻道结束（每当磁头完成一个寻道动作位置1）
     5: =1 故障（表示操作不当，读写或查找动作被禁止）
     6: =1 准备好（当该位和位4均为1时，硬盘准备好接受下一命令）
     7: =1 忙碌  */
outportb(0x1F6,( loop == 0 ? 0xA0:0xB0 )); /*选择硬盘0或1*/
/* IDE(EIDE)硬盘端口1F6（磁头选择寄存器）定义：
   位0~3: 磁头选择
       4: 其值0为第一硬盘，其值为1为第二硬盘
     5~7: =101（不能为其他值） */
outportb(0x1F7, 0x10);
/* 硬盘端口1F7写（状态寄存器），可输入IDE硬盘的控制命令有：
       磁头回0道 1X      001xxxx
       读扇区    20~23   001000LR  注: L=0 正常ECC功能状态
       写扇区    30~33   001100LR      L=1 长状态
       读检验    40~41   0100000R      R=0 可使用错误重试
       格式化磁道  50     01010000      R=1 不使用错误重试
       查找       7X     0111xxxx
       诊断       90     10010000
       设置参数    91     10010001
       读硬盘参数  EC     11101100 */
        while ( inportb(0x1F7) >= 0x80 );
        if ( inportb(0x1F7) != 0x50 )
        {
         if ( hd_number == 0 )
         {
         printf("\b\b&&&&&该系统没有安装硬盘&&&&&*\n");
         exit(0);
         }
        }
        else hd_number++;
}
printf("---------------------------------\n");
printf("系统共安装有%lu个硬盘，取得物理参数如下:\n",hd_number);
printf("---------------------------------\n");

for ( loop=0; loop<hd_number; loop++)
    /* 取硬盘物理参数信息 */
{
outportb( 0x1F6, ( loop == 0 ? 0xA0 : 0xB0 ));
outportb( 0x1F7, 0xEC );              /* 发送“读硬盘参数”指令*/
while ( inportb(0x1F7) != 0x58 );     /* 等待控制送回参数 */
for (info_point=0; info_point!=256; info_point++)
  hd_info[info_point] = inport(0x1F0);/* 读取送回的512字节数据 */
/* 计算硬盘容量 */
hd_size =  (int)( (long)hd_info[1] * (long)hd_info[3] * (long)hd_inf[6] * 512/1000000 );
printf("*********************************\n");
printf("第%u个IDE硬盘容量: %6.2fMb\n\n",loop+1,(float)hd_size);
printf("产品型号--------: %s\n",string(hd_info,27,46));
printf("产品序列号------: %s\n",string(hd_info,10,19));
printf("控制器版本号----: %s\n",string(hd_info,23,26));
printf("控制器类型------: %04X\n",hd_info[20]);
printf("控制器缓冲区(Kb): %6u\n",hd_info[21]*512/1024);
printf("能否双字传送----: %6s\n",hd_info[48] == 0 ? "不能" : "能够" );
printf("ECC检验码长度(b): %6u\n",hd_info[22]);
printf("每次传送扇区数---: %6u\n",hd_info[47]);
printf("物理柱面数-------: %6u\n",hd_info[1]);
printf("物理磁头数-------: %6u\n",hd_info[3]);
printf("每道扇区数-------: %6u\n",hd_info[6]);
/* 取CMOS中有关硬盘信息(INT 13 第8号功能) */
registers.h.ah = 0x8;
registers.h.dl = 0x80 + loop;
int86( 0x13, &registers, &registers );
if ( !registers.x.cflag )       /* 如未置进位C,成功 */
{
cmos_head[loop] = registers.h.dh + 1;     /* 返回CMOS中定义的磁头数 */
cmos_sec[loop] = registers.h.cl & 0x3F;   /* 为0~5为每道扇区数 */
cmos_cyl[loop] = (( registers.h.cl & 0xC0 ) << 2 )+registers.h.ch+2;
  /* 柱面号从0开始，且FDISK保留了最后一个柱面，因而记数+2 */
printf("---------------------------------\n");
printf("CMOS中定义的逻辑柱面数；--: %6u\n",cmos_cyl[loop]);
printf("CMOS中定义的逻辑磁头数；--: %6u\n",cmos_head[loop]);
printf("CMOS中定义的逻辑扇区数；--: %6u\n",cmos_sec[loop]);
printf("---------------------------------\n");
}
printf("*********************************\n");
}
} 