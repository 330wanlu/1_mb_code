#ifndef _TAX_INFO_H
#define _TAX_INFO_H
//#include "common.h"
#include "mb_typedef.h"
#include <string.h>

enum enDeviceType {
    DEVICE_UNKNOW = 0,
    DEVICE_AISINO,  //����ŵ��˰��
    DEVICE_NISEC,   //����˰����
    DEVICE_UKEY,    //˰������UKey

};

enum enCommonFPLX {
    COMMOM_FPLX_ZYFP = 4,   //ר�÷�Ʊ
    COMMOM_FPLX_PTFP = 7,   //��ͨ��Ʊ
    COMMOM_FPLX_HYFP = 9,   //���˷�Ʊ
    COMMOM_FPLX_JDCFP = 5,  //��������Ʊ
    COMMOM_FPLX_JSFP = 25,  //��ʽ��Ʊ
    COMMOM_FPLX_DZFP = 26,  //���ӷ�Ʊ
    COMMOM_FPLX_ESC = 6     //���ֳ���Ʊ
};

enum enAnsinoFPLX {
    AISINO_FPLX_ZYFP = 0,    //ר�÷�Ʊ
    AISINO_FPLX_PTFP = 2,    //��ͨ��Ʊ
    AISINO_FPLX_HYFP = 11,   //���˷�Ʊ
    AISINO_FPLX_JDCFP = 12,  //��������Ʊ
    AISINO_FPLX_JSFP = 41,   //��ʽ��Ʊ
    AISINO_FPLX_DZFP = 51,   //���ӷ�Ʊ
    AISINO_FPLX_ESC = 42     //���ֳ���Ʊ
};


// 0һ��רƱ 2 ʯ���͡�ȼ������ֵ˰ר�÷�Ʊ  ��Ʊʱ 0 һ����Ʊ 8ũ��Ʒ���� 9ũ��Ʒ�չ�
/*ר�÷�Ʊ����*/
enum enZYFP_LX {
    ZYFP_ZYFP,      //һ��ר��Ʊ
    ZYFP_HYSY,      //����ʯ��
    ZYFP_SNY,       //ʯ����
    ZYFP_SNY_DDZG,  //ʯ����_DDZG
    ZYFP_RLY,       //ȼ����
    ZYFP_RLY_DDZG,  //ȼ����_DDZG
    ZYFP_XT_YCL,    //ϡ��_ԭ����
    ZYFP_XT_CCP,    //ϡ��_����Ʒ
    ZYFP_NCP_XS,    //ũ��Ʒ_����
    ZYFP_NCP_SG,    //ũ��Ʒ_�չ�
    ZYFP_JZ_50_15,  //����_50_15
    ZYFP_CEZS,      //�������
    ZYFP_TXF_KDK,   //ͨ�з�_�ɵֿ�
    ZYFP_TXF_BKDK,  //ͨ�з�_���ɵֿ�
    ZYFP_CPY,       //��Ʒ��
    ZYFP_XT_JSJHJ,  //ϡ��_�������Ͻ�
    ZYFP_XT_CPJGF   //ϡ��_��Ʒ�ӹ���
};

//��Ʊ������
enum FPHXZ {
    FPHXZ_SPXX,         //��Ʒ��Ϣ
    FPHXZ_XJXHQD,       //��������嵥
    FPHXZ_SPXX_ZK = 3,  //��Ʒ��Ϣ�ۿ�(���ۿ�)
    FPHXZ_ZKXX,         //�ۿ���Ϣ(�ۿ�)
    FPHXZ_XHQDZK,       //�����嵥�ۿ�
    FPHXZ_XJDYZSFPQD    //�����Ӧ������Ʊ�嵥
};


struct StaticDeviceInfo {
    unsigned short usICCardNo;  //˰�̷ֻ���
    char szDeviceTime[24];      //�豸ʱ�� yyyymmddhhmmss 20191019084029
    char szNicMAC[24];          //��Ʊͨ������MAC��ַ
    char szDeviceID[24];        //�豸���SN
    char szDriverVersion[32];   //�ײ�Ӳ�������汾��
    char szJMBBH[4];            //���ܰ汾��
    char szCommonTaxID[24];     //ͨ����ʾ˰��
    char szCompressTaxID[24];   //ѹ��˰��
    char sz9ByteHashTaxID[24];  // 9λhash˰��
    char szRegCode[12];         //�������
    //˰�̹�˾����,���50������ ������GBK������ռ2���ֽڣ���UTF8������ռ3���ֽ�
    char szComName[256];
    void *pX509Cert;         //���ڿ�Ʊǩ����֤��ָ��
    unsigned char bTrainFP;  //�Ƿ��ղ�����(������ѵ��ҵ)
	unsigned char bTrainFP2;  //�Ƿ�ɽ��������(���Ƶ���˰��)
    HUSB hUSBDevice;         //�豸��libusb���
};
typedef struct StaticDeviceInfo *HDEV;  // replace 'struct StaticDeviceInfo *' to 'HDEV '

struct Spxx {
    char spmc[128];  //��Ʒ����
    char spbh[128];  //��Ʒ���(spbh)=�������(flbm)=��Ʒ����(spbm);ͳһΪ��Ʒ���,����˵���ϳ�
    char spsm[128];   //��Ʒ˰Ŀ
    char ggxh[128];   //����ͺ�
    char jldw[128];   //������λ
    char dj[128];     //����
    char sl[128];     //����
    char je[128];     //���̶�����˰��
    char slv[128];    //˰��
    char se[128];     //˰��
    char fphxz[128];  //��Ʊ������
    char hsjbz[128];  //��˰�۱�־
    char bz[128];     //��ע,��ע�⣬���ﲻ��UTF8���ģ���GBK���Base64
    char xh[128];     //���
    char xthash[128];
    char bhsdj[128];
    char xsyh[128];  //�����Ż� YHZCBS
    char yhsm[128];
    char lslvbs[128];  //��˰�ʱ�ʾ
    char kce[128];     //�����˰�۳���
    char flag[128];
    char spsmmc[128];  //��Ʒ˰Ŀ����
    char zzstsgl[10];  //��ֵ˰�������
    char zsfs[10];     //���շ�ʽ
    struct Spxx *stp_next;

    //�����ֶ�
};

//��������ǵ�����Ϣ���������ʱ�ж����Ʊ������Ҫ����������ʽ
struct Fpxx {
    //---��ʱ��ʵ��
    // int bool_0;
    // int bsbz;  //��˰��־
    // int bsq;
    // int bszt;
    // char ccdw[128];
    // char cd[128];
    // char cllx[128];
    // char clsbdh[128];
    // char cpxh[128];
    // char cyrmc[128];
    // char cyrnsrsbh[128];
    // char czch[128];
    // char dkqymc[128];
    // char dkqysh[128];
    // int dybz;
    // char dy_mb[128];  //��ӡģ��
    // char fdjhm[128];
    // char fhrmc[128];
    // char fhrnsrsbh[128];
    // int flag;
    // char fpqqlsh[128];
    // char hgzh[128];
    // char hxm[128];
    // int hzfw;
    // long invQryNo;
    // int isBlankWaste;
    // int isNewJdcfp;
    // char jkzmsh[128];
    // char jmbbh[64];
    // unsigned long keyFlagNo;
    // char kpsxh[128];
    // char qyd[128];
    // char rdmByte[128];
    // char sbbz[128];
    // char sccjmc[128];
    // char sfzhm[128];
    // char shrmc[128];
    // char shrnsrsbh[128];
    // char sjdh[128];
    // char wspzhm[128];
    // char xcrs[128];
    // int xfbz[128];
    // char xfyh[128];
    // char xsdjbh[128];
    // char yshwxx[128];
    // char zfsj[128];
    // char zgswjgdm[128];
    // char zgswjgmc[128];

    //-------ʵ��
    HDEV pDevInfo;              //˰�̻��������;�̬������Ϣ
    struct Spxx *stp_MxxxHead;  //��Ʒ��ϸ��Ϣ
    char bmbbbh[128];           //�����汾��
    char bz[512];               //��ע;UTF8����
    char fhr[128];              //������
    char fpdm[128];             //��Ʊ����  szInvTypecode
    char fphm[128];             //��Ʊ����  szCurrentInvNum
    char fpendhm[128];          //��Ʊ���������  szInvEndNum
    char fplx[12];              //��Ʊ���ͣ�004=רƱ 005 006 007=��Ʊ 009 025 026=��Ʊ��
    char gfdzdh[256];           //������ַ�绰
    char gfmc[256];             //��������
    char gfsh[128];             //����˰��
    char gfyhzh[256];           //���������˺�
    char je[128];               //���
    char jqbh[32];              //�������
    char jym[256];              //У����
    char kpr[128];              //��Ʊ��
    char kpsj_standard[24];     //��Ʊʱ��ͳһ��׼��ʽ
                             // 20190719145100,������ʽ����ͨ��TranslateStandTime����ת������
    char kpsj_F1[24];           //��Ʊʱ���ʽ1 20190719 14:51:00
    char kpsj_F2[24];           //��Ʊʱ���ʽ2 2019-07-19 14:51:00
    char kpsj_F3[24];           //��Ʊʱ���ʽ3 20190719145100
    char mw[128];               //����
    char se[128];               //˰��
    char sign[512];             //ǩ�� Base64
    char skr[128];              //�տ���
    char slv[128];              //˰��
    char ssyf[12];              //�����·�
    char xfdzdh[256];           //������ַ�绰
    char xfmc[256];             //��������
    char xfsh[128];             //����˰��
    char xfyhzh[256];           //���������˺�
    uint8 zyfpLx;       //ר�÷�Ʊ����
    char zyspmc[128];           //ר����Ʒ����
    char kprjbbh[128];          //��Ʊ����汾��
    char qdbj[128];             //�嵥���
    char spfmc[128];            //
    char spfnsrsbh[128];        //
    int isRed;                  //�Ƿ��Ʊ
    char dkbdbs[128];           //
    char yddk[128];             //Զ�˶˿ڣ�
    char redNum[32];            //ר�÷�Ʊ��Ʊ֪ͨ�����
    int zfbz;                   //���ϱ�־
    char blueFpdm[128];         //��Ʊ��Ʊ����
    char blueFphm[128];         //��Ʊ��Ʊ����
    char jshj[20];              //��˰�ϼ�
    char hsbz[12];              //��˰��־
    char hzfw[12];              //���ַ�α
    char dslp[12];              //��˰��Ʊ
    char zhsl[12];              //�ۺ�˰��
    char hsjbz[128];            //��˰�۱�־
    char zyspsmmc[128];         //ר����Ʒ˰Ŀ����
    int spsl;                   //��Ʒ����
    char fpqqlsh[128];          //��Ʊ������ˮ��
    char kplx[12];              //��Ʊ����,kplx 0/1, blue or red
    uint8 fplx_aisino;  //�����Զ��巢Ʊ����
    char tspz[12];  //����Ʊ��רƱʱ����zyfpLx�ظ���Ŀǰ���������ã�   0һ��רƱ 2
                    //ʯ���͡�ȼ������ֵ˰ר�÷�Ʊ  ��Ʊʱ 0 һ����Ʊ 8ũ��Ʒ���� 9ũ��Ʒ�չ�
    char yysbz[50];  //Ӫҵ˰��־
    char zfsj[24];   //����ʱ��,��׼ʱ���ʽyyyymmddhhmmss
    int kpjh;        //��Ʊ����
    //��Ʊ�ϱ��ɹ��󣬻�ȡ���������кţ�����ͨ�������кŲ�ѯ��������Ʊ�ϱ���Ĵ���״̬
    char slxlh[64];
    //��ַ�����ţ���Ʊ�ϱ��ɹ���˰�̷��صĺ��룬�ú��뽫���ں���˰�̶��ڸտ���Ʊ�Ĳ��ң�����Ҫ
    unsigned int dzsyh;
    char aisino_jmbbh[12];   //���ܰ汾��
    char aisino_driver[32];  //��˰�豸�ײ�汾��
    char aisino_inner[32];
    char aisino_xfver[32];
    char aisino_bmbbh[12];
    uint8 bIsUpload;  //==1 �Ѿ��ϱ���==0 ��δ�ϱ�

    //�����ֶ�
};
typedef struct Fpxx *HFPXX;

#define DF_MAX_INV_TYPE		7
#define DF_MAX_INV_COILS	10
#define DF_MAX_SUPPORT_SLV	6
#define DF_MAX_INV_LEN		3*1024*1024



//˰�̻�����Ϣ
struct _plate_basic_info
{
	int result;          // result�Ƿ���״̬�룬0 ��˰�����ƺ�˰�Ŷ�û�� 1 �ܶ�����˰������ 2 �ܶ�����˰��˰�� 3 ���ܶ��� 
	unsigned char ca_name[200];
	unsigned char ca_number[50];
	unsigned char plate_num[20];
	unsigned short plate_type;
	unsigned short extension;
};

//˰��˰�������Ϣ
struct _plate_tax_info
{
	int inv_type_num;
	unsigned char area_code[20];		//�������
	unsigned char inv_type[10];			//֧�ֵķ�Ʊ���� 4 5 6 7 25 26
	unsigned char tax_office_code[20];	//����˰����ش���
	unsigned char tax_office_name[100];	//����˰���������
	unsigned char startup_date[20];		//��������
};

//˰��ĳƱ�ּ����Ϣ
struct _monitor_info
{
	unsigned short state;		//�ṹ��״̬
	unsigned char kpjzsj[19];	//��Ʊ��ֹ����
	unsigned char bsqsrq[19];	//������ʼ����
	unsigned char bszzrq[19];	//������ֹ����
	unsigned char zxbsrq[19];	//���±�˰����
	unsigned char dzkpxe[20];	//���ſ�Ʊ�޶�
	unsigned char lxzsljje[20];	//���������ۻ����
	unsigned int lxsc;			//����ʱ��
	unsigned char lxzssyje[20];//��������ʣ����
};

//˰��δʹ�÷�Ʊ����Ϣ
struct _invoice_coil
{
	unsigned short state;		//�ṹ��״̬
	unsigned char fplbdm[20];	//��Ʊ������
	unsigned int fpqshm;		//��Ʊ��ʼ����
	unsigned int fpzzhm;		//��Ʊ��ֹ����
	unsigned int fpsyfs;		//��Ʊʣ�����
	unsigned char fpgmsj[19];	//����ʱ��
	unsigned int fpgmsl;		//��������
};

//˰��ĳƱ��δʹ�÷�Ʊ����Ϣ
struct _invoice_coils_info
{
	unsigned short state;		//�ṹ��״̬
	unsigned int zsyfpfs;		//��ʣ�෢Ʊ����
	unsigned short wsyfpjsl;	//δʹ�÷�Ʊ������
	unsigned char dqfpdm[20];   //��ǰ��Ʊ����
	unsigned int  dqfphm;		//��ǰ��Ʊ����
	struct _invoice_coil invoice_coil[10];
};



//˰��ȫ����Ȩ˰����Ϣ
struct _plate_support_slv
{
	unsigned short state;						//�ṹ��״̬
	unsigned short slv_sl;
	unsigned int slv[DF_MAX_SUPPORT_SLV];		//��Ȩ˰��֧������
};

//����Ʊ��Ϣ
struct _invoice_offline_info
{
	unsigned short state;						//�ṹ��״̬
	int offline_num;
};

//˰����������Ϣ�ṹ��
struct _invoice_type_infos
{
	unsigned short state;						//�ṹ��״̬
	unsigned char fplxdm;						//��Ʊ���ʹ����ڴ���Ź̶� �ڴ�0 4רƱ  �ڴ�1=5������  �ڴ�2=6���ֳ�   �ڴ�3=7��Ʊ  �ڴ�4=9����   �ڴ�5=25��Ʊ  �ڴ�6=26��Ʊ
	struct _monitor_info			monitor_info;			//˰��ĳƱ�ּ����Ϣ
	struct _invoice_coils_info		invoice_coils_info;		//˰��ĳƱ��δʹ�÷�Ʊ����Ϣ
	struct _plate_support_slv		plate_support_slv;		//˰��ȫ����Ȩ˰����Ϣ
	struct _plate_support_slv		plate_support_noslv;	//˰�̲���˰˰����Ϣ
	struct _invoice_offline_info	invoice_offline_info;	//����Ʊ��Ϣ
};

//˰��ȫ����Ϣ�ṹ��
struct _plate_infos
{
	struct _plate_basic_info		plate_basic_info;		//˰�̻�����Ϣ
	struct _plate_tax_info			plate_tax_info;			//˰��˰�������Ϣ
	struct _invoice_type_infos		invoice_type_infos[DF_MAX_INV_TYPE+1];//���֧��7��Ʊ�� 004רƱ 005������ 006���ֳ� 007��Ʊ 009���� 025��Ʊ 026��Ʊ
};

//��Ʊ���߽��
struct _make_invoice_result
{
	unsigned char fplxdm;	//��Ʊ����
	unsigned char fpdm[20];	//��Ʊ����
	unsigned char fphm[20];	//��Ʊ����
	unsigned char kpsj[20];	//��Ʊʱ��
	unsigned char hjje[20];	//�ϼƽ��
	unsigned char hjse[20]; //�ϼ�˰��
	unsigned char jshj[20]; //��˰�ϼ�
	unsigned char jym[22];	//У����
	unsigned char mwq[110];	//������
	unsigned char fpqqlsh[55];//��Ʊ������ˮ��
	unsigned char bz[500];
	int sfsylsh;//�Ƿ�ʹ����ˮ��
	int fpdzsyh;
	char *scfpsj;//Ҫ�ϴ��ķ�Ʊ����,����Ϊzlibѹ������
	unsigned char errinfo[500];
};


//��Ʊ���Ͻ��
struct _cancel_invoice_result
{
	unsigned char fplxdm;	//��Ʊ����
	unsigned char fpdm[20];	//��Ʊ����
	unsigned char fphm[20];	//��Ʊ����
	unsigned char zfsj[20]; //����ʱ��
	unsigned char jym[22];	//У����
	unsigned char mwq[110];	//������
	unsigned char errinfo[500];
};

//��������
struct _copy_report
{
	unsigned char fplxdm;
	unsigned char cbsj[20];
	unsigned char hzksrq[20];	//���ܿ�ʼ����
	unsigned char hzjzrq[20];	//���ܽ�ֹ����
	unsigned int zcfpkjfs;		//������Ʊ���߷���
	unsigned int hpkjfs;		//��Ʊ���߷���
	unsigned int zcfpzffs;		//������Ʊ���Ϸ���
	unsigned int hpzffs;		//��Ʊ���Ϸ���
	unsigned int kbfs;			//�հ׷���
	unsigned char zcfpkjje[20]; //������Ʊ���߽��
	unsigned char zcfpse[20];   //������Ʊ˰��
	unsigned char zcfpjshj[20];	//������Ʊ��˰�ϼ�
	unsigned char hpkjje[20];	//��Ʊ���߽��
	unsigned char hpse[20];		//��Ʊ˰��
	unsigned char hpjshj[20];	//��Ʊ��˰�ϼ�
};


#endif