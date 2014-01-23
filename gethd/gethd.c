#include <dos.h>
#include <stdio.>
#include <stdlib.h>
#include <conio.h>
#include <bios.h>

/*�����ֱ�ʾ���ַ���ת���ɿ���ʾ��ASCII�ַ�����ʽ*/
char *string( unsigned int in_data[], int start, int end )
{
static char ret_str[40];
int loop,loopl;
for ( loop = start,loopl = 0; loop <= end; loop++ )
{
ret_str[loopl++] = (char)(in_data[loop]/256);/*ȡ�ֵĸ��ֽ�*/
ret_str[loopl++] = (char)(in_data[loop]%256);/*ȡ�ֵĵ��ֽ�*/
}
ret_str[loopl] = ' \0';   /*�ַ�����0����*/
return(ret_str);
}

void main( void )
{
unsigned int hd_info[256]; /*Ӳ����������ݴ���*/
unsigned int info_point;   /*��Ϣ������ָ��*/
unsigned int loop;         /*ѭ��ָ��*/
unsigned int hd_number = 0;/*ϵͳ����Ӳ����*/
unsigned int hd_size;      /*Ӳ������*/
union REGS registers;
/*���±�������ΪCMOS SETUP�����õ��߼���ʽ������������ͷ���������� */
unsigned int cmos_xyl[2], cmos_head[2], cmos_sec[2];

clrscr();
printf("\n***IDE(EIDE)Ӳ�������߼���ʽ���������***\n");
for ( loop = 0; loop < 2; loop++)
{
while (inportb(0x1f7) >= 0x80; /*�ȴ�Ӳ�̿�����׼����*/
/* Ӳ�̶˿�1F7����״̬�Ĵ��������壺
   λ0��=1 ����ִ�г���
     1: =1 �յ�������ÿ��������תһ�ܣ���λ��1��
     2: =1 ECC������󣨶�ȡ�����ݱ�ECC�㷨������
     3: =1 ���������ڶ�дʱ�����������������������������
     4: =1 Ѱ��������ÿ����ͷ���һ��Ѱ������λ��1��
     5: =1 ���ϣ���ʾ������������д����Ҷ�������ֹ��
     6: =1 ׼���ã�����λ��λ4��Ϊ1ʱ��Ӳ��׼���ý�����һ���
     7: =1 æµ  */
outportb(0x1F6,( loop == 0 ? 0xA0:0xB0 )); /*ѡ��Ӳ��0��1*/
/* IDE(EIDE)Ӳ�̶˿�1F6����ͷѡ��Ĵ��������壺
   λ0~3: ��ͷѡ��
       4: ��ֵ0Ϊ��һӲ�̣���ֵΪ1Ϊ�ڶ�Ӳ��
     5~7: =101������Ϊ����ֵ�� */
outportb(0x1F7, 0x10);
/* Ӳ�̶˿�1F7д��״̬�Ĵ�������������IDEӲ�̵Ŀ��������У�
       ��ͷ��0�� 1X      001xxxx
       ������    20~23   001000LR  ע: L=0 ����ECC����״̬
       д����    30~33   001100LR      L=1 ��״̬
       ������    40~41   0100000R      R=0 ��ʹ�ô�������
       ��ʽ���ŵ�  50     01010000      R=1 ��ʹ�ô�������
       ����       7X     0111xxxx
       ���       90     10010000
       ���ò���    91     10010001
       ��Ӳ�̲���  EC     11101100 */
        while ( inportb(0x1F7) >= 0x80 );
        if ( inportb(0x1F7) != 0x50 )
        {
         if ( hd_number == 0 )
         {
         printf("\b\b&&&&&��ϵͳû�а�װӲ��&&&&&*\n");
         exit(0);
         }
        }
        else hd_number++;
}
printf("---------------------------------\n");
printf("ϵͳ����װ��%lu��Ӳ�̣�ȡ�������������:\n",hd_number);
printf("---------------------------------\n");

for ( loop=0; loop<hd_number; loop++)
    /* ȡӲ�����������Ϣ */
{
outportb( 0x1F6, ( loop == 0 ? 0xA0 : 0xB0 ));
outportb( 0x1F7, 0xEC );              /* ���͡���Ӳ�̲�����ָ��*/
while ( inportb(0x1F7) != 0x58 );     /* �ȴ������ͻز��� */
for (info_point=0; info_point!=256; info_point++)
  hd_info[info_point] = inport(0x1F0);/* ��ȡ�ͻص�512�ֽ����� */
/* ����Ӳ������ */
hd_size =  (int)( (long)hd_info[1] * (long)hd_info[3] * (long)hd_inf[6] * 512/1000000 );
printf("*********************************\n");
printf("��%u��IDEӲ������: %6.2fMb\n\n",loop+1,(float)hd_size);
printf("��Ʒ�ͺ�--------: %s\n",string(hd_info,27,46));
printf("��Ʒ���к�------: %s\n",string(hd_info,10,19));
printf("�������汾��----: %s\n",string(hd_info,23,26));
printf("����������------: %04X\n",hd_info[20]);
printf("������������(Kb): %6u\n",hd_info[21]*512/1024);
printf("�ܷ�˫�ִ���----: %6s\n",hd_info[48] == 0 ? "����" : "�ܹ�" );
printf("ECC�����볤��(b): %6u\n",hd_info[22]);
printf("ÿ�δ���������---: %6u\n",hd_info[47]);
printf("����������-------: %6u\n",hd_info[1]);
printf("�����ͷ��-------: %6u\n",hd_info[3]);
printf("ÿ��������-------: %6u\n",hd_info[6]);
/* ȡCMOS���й�Ӳ����Ϣ(INT 13 ��8�Ź���) */
registers.h.ah = 0x8;
registers.h.dl = 0x80 + loop;
int86( 0x13, &registers, &registers );
if ( !registers.x.cflag )       /* ��δ�ý�λC,�ɹ� */
{
cmos_head[loop] = registers.h.dh + 1;     /* ����CMOS�ж���Ĵ�ͷ�� */
cmos_sec[loop] = registers.h.cl & 0x3F;   /* Ϊ0~5Ϊÿ�������� */
cmos_cyl[loop] = (( registers.h.cl & 0xC0 ) << 2 )+registers.h.ch+2;
  /* ����Ŵ�0��ʼ����FDISK���������һ�����棬�������+2 */
printf("---------------------------------\n");
printf("CMOS�ж�����߼���������--: %6u\n",cmos_cyl[loop]);
printf("CMOS�ж�����߼���ͷ����--: %6u\n",cmos_head[loop]);
printf("CMOS�ж�����߼���������--: %6u\n",cmos_sec[loop]);
printf("---------------------------------\n");
}
printf("*********************************\n");
}
} 