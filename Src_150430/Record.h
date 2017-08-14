
#ifndef __RECORD_H__
#define __RECORD_H__

//�豸��Ϣ�洢��ַ
#define SYSTEM_STATE_ADDR       0x00
#define TERMINAL_INDEX_ADDR     0x01
#define TERMINAL_TYPE_ADDR      0x05
#define TERMINAL_INFO_LEN       0x06
#define TERMINAL_INFO_ADDR      0x07
#define TIME_SPACE_ADDR         0x30

//DTU��ʶ����
#define DTU_INDEX_ADDR          0x31   //11���ֽڿռ�

//����汾��
#define FW_VER_ADDR             0x3c   //�̼��汾��ǰ��4byte�ֱ�洢
#define DEBUG_EN_ADDR           0x3d   //debug���ء�1 --- on��0 ---off

//��ʼ��ַ��Ŷ˿ں�
#define DTU_MAIN_COM_ADDR       0x40
#define DTU_MAIN_TCP_ADDR       0x42
#define DTU_MAIN_APN_ADDR       0x43
#define DTU_MAIN_NAME_LEN       0x44
#define DTU_MAIN_NAME_ADDR      0x45  //�û���Ϊ���16���ַ�
#define DTU_MAIN_PASS_LEN       0x55
#define DTU_MAIN_PASS_ADDR      0x56  //�������Ϊ16���ַ�
#define DTU_MAIN_LEN_ADDR       0x66
#define DTU_MAIN_IP_ADDR        0x67

//��ʼ��ַ��ʼ�ȴ�������BYTE�Ķ˿ںţ���������IP��������
//������󳤶�Ϊ60
#define DTU_SUB1_COM_ADDR       0xB0
#define DTU_SUB1_EN_ADDR        0xB2
#define DTU_SUB1_LEN_ADDR       0xB3
#define DTU_SUB1_IP_ADDR        0xB4

#define DTU_SUB2_COM_ADDR       0xF0
#define DTU_SUB2_EN_ADDR        0xF2
#define DTU_SUB2_LEN_ADDR       0xF3
#define DTU_SUB2_IP_ADDR        0xF4

#define DTU_SUB3_COM_ADDR       0x130
#define DTU_SUB3_EN_ADDR        0x132
#define DTU_SUB3_LEN_ADDR       0x133
#define DTU_SUB3_IP_ADDR        0x134

#define DTU_SUB4_COM_ADDR       0x170
#define DTU_SUB4_EN_ADDR        0x172
#define DTU_SUB4_LEN_ADDR       0x173
#define DTU_SUB4_IP_ADDR        0x174



/********************************************************************************
 * �������ƣ�RecordInit.c
 * �������ܣ���ʼ����¼��Ϣ�������豸��ţ��豸��Ϣ��
 * 
 * ���룺 ��
 * ���أ� ��
 *  
 *******************************************************************************/
extern void RecordInit(void);

/********************************************************************************
 * �������ƣ�RecordRun
 * �������ܣ����մ��ڷ��͵��������ݣ�����ʾ����ʾ����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void RecordRun(void);


//����������ػ�״̬
extern BOOL CheckPowerDown(void);

#endif