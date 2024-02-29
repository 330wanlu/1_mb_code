#ifndef DF_DEC_FILE
	#define DF_DEC_FILE
	#include "mb_typedef.h"
	#include "cJSON.h"
	#include "_log.h"
	#include <openssl/err.h>
	#include <openssl/md5.h>
	#include <openssl/rsa.h>
	#include <openssl/ssl.h>
	#include "_des.h"
	#include "_zlib.h"
	#include "_base64.h"
	//#include "_timer.h"
/*--------------------------------程序初始定义------------------------------------*/
#ifdef _dec_file
	#define out(s,arg...)               log_out("all.c",s,##arg)
#else
#endif



struct _public_key
{
	char key_value[2048];
};

struct _private_key
{
	char key_value[2048];
};

struct _key_sum
{
	struct _public_key		public_key[10];
	struct _private_key		private_key[10];
};

struct _enc_file_s
{
	int key_num_s;
	char sign_string[4096];
	int key_num_t;
	char enc_key[200];
	int main_len;
	char *main_body;
	char des_key[100];
};


	
extern int mb_dec_file_function(unsigned char *key_zlib_json, char *indata, int inlen, char **outdata, int *outdatalen);
extern int mb_enc_singn_chect_function(unsigned char *key_zlib_json, char **outdata, int *outdatalen);

extern int mb_enc_get_mqtt_connect_info_json(uint8 *key_zlib_json, char *ter_id, char **json_data);

//读取密钥
//
extern RSA* createRSA(unsigned char*key, int publi);
//公钥加密，私钥解密
//
extern int public_encrypt(unsigned char*data, int data_len, unsigned char*key, unsigned char*encrypted);
extern int private_decrypt(unsigned char*enc_data, int data_len, unsigned char*key, unsigned char*decrypted);

//私钥加密，公钥解密
//
extern int private_encrypt(unsigned char*data, int data_len, unsigned char*key, unsigned char*encrypted);
extern int public_decrypt(unsigned char*enc_data, int data_len, unsigned char*key, unsigned char*decrypted);
//私钥签名，公钥验签
//
extern int private_sign(const unsigned char *in_str, unsigned int in_str_len, unsigned char *outret, unsigned int *outlen, unsigned char*key);
extern int public_verify(const unsigned char *in_str, unsigned int in_len, unsigned char *outret, unsigned int outlen, unsigned char*key);

extern int md5_value(char *src, int src_len, char *dest);

#ifdef _dec_file
static int analysis_key_value(char *key_json, struct _key_sum *key_sum);
static int analysis_enc_file(char *in_json, struct _enc_file_s *enc_file_s);
static int verify_file_sign(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s);
static int dec_key_value(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s);
#endif










#endif