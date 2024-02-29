#define _dec_file
#include "_dec_file.h"


//读取密钥
//
RSA* createRSA(unsigned char*key, int publi)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);
	if (keybio == NULL)
	{
		out("Failed to create key BIO\n");
		return NULL;
	}

	if (publi)
	{
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	}
	else
	{
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	}
	if (rsa == NULL)
	{
		out("Failed to create RSA\n");
	}

	return rsa;
}


//公钥加密，私钥解密
//
int public_encrypt(unsigned char*data, int data_len, unsigned char*key, unsigned char*encrypted)
{
	RSA* rsa = createRSA(key, 1);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	int result = RSA_public_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);
	RSA_free(rsa);
	return result;
}

int private_decrypt(unsigned char*enc_data, int data_len, unsigned char*key, unsigned char*decrypted)
{
	RSA* rsa = createRSA(key, 0);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_PADDING);
	RSA_free(rsa);
	return result;
}

//私钥加密，公钥解密
//
int private_encrypt(unsigned char*data, int data_len, unsigned char*key, unsigned char*encrypted)
{
	RSA* rsa = createRSA(key, 0);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	int result = RSA_private_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);
	RSA_free(rsa);
	return result;
}
int public_decrypt(unsigned char*enc_data, int data_len, unsigned char*key, unsigned char*decrypted)
{
	RSA* rsa = createRSA(key, 1);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	int result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_PADDING);
	RSA_free(rsa);
	return result;
}

//私钥签名，公钥验签
//
int private_sign(const unsigned char *in_str, unsigned int in_str_len, unsigned char *outret, unsigned int *outlen, unsigned char*key)
{
	RSA* rsa = createRSA(key, 0);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	int result = RSA_sign(NID_md5, in_str, in_str_len, outret, outlen, rsa);
	RSA_free(rsa);
	if (result != 1)
	{
		out("sign error\n");
		return -1;
	}
	return result;
}
int public_verify(const unsigned char *in_str, unsigned int in_len, unsigned char *outret, unsigned int outlen, unsigned char*key)
{
	int result;
	RSA* rsa = createRSA(key, 1);
	if (rsa == NULL)
	{
		out("createRSA error\n");
		return -1;
	}
	result = RSA_verify(NID_md5, in_str, in_len, outret, outlen, rsa);
	RSA_free(rsa);
	if (result != 1)
	{
		out("verify error\n");
		return -1;
	}
	return result;
}




static int analysis_key_value(char *key_json, struct _key_sum *key_sum)
{
	int size;
	int i;
	char num[10];
	cJSON *arrayItem, *item;
	cJSON *root = cJSON_Parse(key_json);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object_public = cJSON_GetObjectItem(root, "public");
	if (object_public == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_public);
	if (size < 1)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	for (i = 0; i < size; i++)
	{
		int num_i;
		arrayItem = cJSON_GetArrayItem(object_public, i);
		if (arrayItem == NULL)
		{
			out("Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		memset(num, 0, sizeof(num));
		item = cJSON_GetObjectItem(arrayItem, "num");
		if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
			out("解析公钥密钥组序号失败,i=%d\n", i);
			goto End;
		}
		num_i = atoi(num);
		char ca_tmp[2048] = {0};
		item = cJSON_GetObjectItem(arrayItem, "key");
		if (get_json_value_can_not_null(item, ca_tmp, 0, 2000) < 0) {
			out("解析公钥密钥组值失败,i=%d\n", i);
			goto End;
		}
		int count;
		int last = 0;
		char tmp[65] = { 0 };
		char real_key[2048] = { 0 };
		int j;

		count = strlen(ca_tmp) / 64;
		last = strlen(ca_tmp) % 64;
		for (j = 0; j < count; j++)
		{
			memset(tmp, 0, 64);
			memcpy(tmp, ca_tmp + (64 * j), 64);
			sprintf(real_key + strlen(real_key), "%s\n", tmp);
		}
		memset(tmp, 0, 64);
		memcpy(tmp, ca_tmp + (64 * count), last);
		sprintf(real_key + strlen(real_key), "%s", tmp);
		sprintf(key_sum->public_key[num_i - 1].key_value, "-----BEGIN PUBLIC KEY-----\n%s\n-----END PUBLIC KEY-----", real_key);
		//out("第%d组公钥为%s\n", num_i, key_sum->public_key[num_i - 1].key_value);
	}
	cJSON *object_pravite = cJSON_GetObjectItem(root, "private");
	if (object_pravite == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_pravite);
	if (size < 1)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	for (i = 0; i < size; i++)
	{
		int num_i;
		arrayItem = cJSON_GetArrayItem(object_pravite, i);
		if (arrayItem == NULL)
		{
			out("Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		memset(num, 0, sizeof(num));
		item = cJSON_GetObjectItem(arrayItem, "num");
		if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
			out("解析私钥密钥组序号失败,i=%d\n", i);
			goto End;
		}
		num_i = atoi(num);
		char ca_tmp[2048] = { 0 };
		item = cJSON_GetObjectItem(arrayItem, "key");
		if (get_json_value_can_not_null(item, ca_tmp, 0, 2000) < 0) {
			out("解析私钥密钥组值失败,i=%d\n", i);
			goto End;
		}
		int count;
		int last = 0;
		char tmp[65] = { 0 };
		char real_key[2048] = { 0 };
		int j;

		count = strlen(ca_tmp) / 64;
		last = strlen(ca_tmp) % 64;
		for (j = 0; j < count; j++)
		{
			memset(tmp, 0, 64);
			memcpy(tmp, ca_tmp + (64 * j), 64);
			sprintf(real_key + strlen(real_key), "%s\n", tmp);
		}
		memset(tmp, 0, 64);
		memcpy(tmp, ca_tmp + (64 * count), last);
		sprintf(real_key + strlen(real_key), "%s", tmp);
		sprintf(key_sum->private_key[num_i - 1].key_value, "-----BEGIN RSA PRIVATE KEY-----\n%s\n-----END RSA PRIVATE KEY-----", real_key);
		//out("第%d组私钥为%s\n", num_i, key_sum->private_key[num_i - 1].key_value);
	}
End:
	cJSON_Delete(root);
	return 0;

}

static int analysis_enc_file(char *in_json, struct _enc_file_s *enc_file_s)
{
	char num[10];
	int ret = -1;
	cJSON *item;
	cJSON *root = cJSON_Parse(in_json);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}

	memset(num, 0, sizeof(num));
	item = cJSON_GetObjectItem(root, "key_num_s");
	if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
		out("解析服务器密钥组序号失败\n");
		goto End;
	}
	enc_file_s->key_num_s = atoi(num);

	memset(num, 0, sizeof(num));
	item = cJSON_GetObjectItem(root, "key_num_t");
	if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
		out("解析终端密钥组序号失败\n");
		goto End;
	}
	enc_file_s->key_num_t = atoi(num);

	memset(num, 0, sizeof(num));
	item = cJSON_GetObjectItem(root, "main_len");
	if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
		out("解析加密主体长度失败\n");
		goto End;
	}
	enc_file_s->main_len = atoi(num);

	item = cJSON_GetObjectItem(root, "sign_string");
	if (get_json_value_can_not_null(item, enc_file_s->sign_string, 0, 256) < 0) {
		out("解析签名串失败\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "enc_key");
	if (get_json_value_can_not_null(item, enc_file_s->enc_key, 0, 256) < 0) {
		out("解析签名串失败\n");
		goto End;
	}

	
	enc_file_s->main_body = calloc(1,10*1024*1024);
	item = cJSON_GetObjectItem(root, "main_body");
	if (get_json_value_can_not_null(item, enc_file_s->main_body, 0, 10 * 1024 * 1024) < 0) {
		out("解析加密主体失败\n");
		free(enc_file_s->main_body);
		enc_file_s->main_body = NULL;
		goto End;
	}
	ret = 0;
End:
	cJSON_Delete(root);
	return ret;

}

int md5_value(char *src, int src_len,char *dest)
{
	char outmd[16] = { 0 };
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, src, src_len);
	MD5_Final((uint8 *)outmd, &ctx);

	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		sprintf(dest + i * 2, "%02X", (unsigned char)outmd[i]);
	}
	return 0;
}

//验证签名
static int verify_file_sign(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s)
{
	MD5_CTX ctx;
	unsigned char outmd[16] = { 0 };
	unsigned char signret[4098] = { 0 };
	unsigned int siglen;
	int ret;
	MD5_Init(&ctx);
	MD5_Update(&ctx, enc_file_s->main_body, strlen(enc_file_s->main_body));
	MD5_Final(outmd, &ctx);

	siglen = calc_decode_length(enc_file_s->sign_string);
	EVP_DecodeBlock((uint8 *)signret, (const uint8 *)enc_file_s->sign_string, strlen(enc_file_s->sign_string));
	//out("sign data =[%s]\n", signret);
	ret = public_verify((const unsigned char*)outmd, 16, signret, siglen, (unsigned char *)key_sum->public_key[enc_file_s->key_num_s-1].key_value);
	if (ret != 1)
	{
		out("verify failed =[%d]\n", ret);
		return -1;
	}
	//out("verify success =[%d]\n", ret);
	return 0;
}

//生成签名
static int generate_file_sign(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s)
{
	MD5_CTX ctx;
	unsigned char outmd[16] = { 0 };
	unsigned char signret[4098] = { 0 };
	unsigned int tersign_len;
	MD5_Init(&ctx);
	MD5_Update(&ctx, enc_file_s->main_body, strlen(enc_file_s->main_body));
	MD5_Final(outmd, &ctx);
	
	private_sign((uint8 *)outmd, 16, (uint8 *)signret, (unsigned int *)&tersign_len, (uint8 *)key_sum->private_key[enc_file_s->key_num_s - 1].key_value);
	EVP_EncodeBlock((uint8 *)enc_file_s->sign_string, (const uint8 *)signret, tersign_len);

	return 0;
}

//解密对称密钥数据
static int dec_key_value(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s)
{

	unsigned int siglen;
	unsigned char encrypted[4098] = { 0 };
	//char enc_base[] = "LB922pGHW9vyFkeObAw2pd7iLMfQ2LO+sKA3jimmErNGdv/H5v6KiG5KFRbmdXn6MIIJla3Hd8sVs3q2SnuRcVFNtumbxj+Gu8L+Ex+TF6Zd++SGqApt5Q76X1HmAHbbtGxMBdrYDbd2+OcRYBEyymwTEBXLtODgD9sRFzLQUAc=";
	siglen = calc_decode_length(enc_file_s->enc_key);
	EVP_DecodeBlock((uint8 *)encrypted, (const uint8 *)enc_file_s->enc_key, strlen(enc_file_s->enc_key));

	int decrypted_length = private_decrypt((unsigned char*)encrypted, siglen, (unsigned char*)key_sum->private_key[enc_file_s->key_num_t-1].key_value, (unsigned char *)enc_file_s->des_key);
	if (decrypted_length == -1)
	{
		out("decrypted error \n");
		return -1;
	}
	//out("DecryptedText =%s\n", enc_file_s->des_key);
	//out("DecryptedLength =%d\n", decrypted_length);
	return 0;
}

//加密对称密钥数据
static int enc_key_value(struct _key_sum *key_sum, struct _enc_file_s *enc_file_s)
{
	int encdata_len;
	char encdata_tmp[1024] = { 0 };

	encdata_len = public_encrypt((uint8 *)enc_file_s->des_key, strlen(enc_file_s->des_key), (uint8 *)key_sum->public_key[enc_file_s->key_num_t - 1].key_value, (uint8 *)encdata_tmp);
	EVP_EncodeBlock((uint8 *)enc_file_s->enc_key, (const uint8 *)encdata_tmp, encdata_len);
	return 0;
}

//加密主体加密文件
static int enc_file_main_body(struct _enc_file_s *enc_file_s, char *indata)
{
	char src_zlib[2048] = { 0 };
	int src_lib_len;
	asc_compress_base64((uint8 *)indata, strlen(indata), (uint8 *)src_zlib, &src_lib_len);
	int buf_len = strlen(src_zlib);
	int zlib_len = strlen(src_zlib);
	int buyu = buf_len % 8;
	if (buyu > 0)
	{
		buf_len = buf_len + 8 - buyu;
	}
	else
	{
		buf_len += 8;
		buyu = 8;
	}
	char src_pkcs7[2048] = {0};
	memset(src_pkcs7, buyu, sizeof(src_pkcs7));
	memcpy(src_pkcs7, src_zlib, zlib_len);

	des3_in((uint8 *)src_pkcs7, buf_len, (uint8 *)enc_file_s->des_key);
	enc_file_s->main_body = calloc(1, 2 * 1024);

	EVP_EncodeBlock((uint8 *)enc_file_s->main_body, (const uint8 *)src_pkcs7, buf_len);
	return 0;
}


//解密主体加密文件
static int dec_file_main_body(struct _enc_file_s *enc_file_s, char **outdata, int *outdatalen)
{
	int siglen;
	int result;
	if (enc_file_s->main_body == NULL)
	{
		return -1;
	}
	int des_len = strlen(enc_file_s->main_body);
	char *des_data = calloc(1, des_len);
	if (des_data == NULL)
	{
		return -2;
	}
	siglen = calc_decode_length(enc_file_s->main_body);
	EVP_DecodeBlock((uint8 *)des_data, (const uint8 *)enc_file_s->main_body, strlen(enc_file_s->main_body));
	
	des3_out((unsigned char *)des_data, siglen, (unsigned char *)enc_file_s->des_key);

	char *src_data = calloc(1, enc_file_s->main_len+1000);
	if (src_data == NULL)
	{
		free(des_data);
		return -3;
	}
	int src_len = enc_file_s->main_len + 1000;
	//memcpy(src_data, des_data, siglen);
	result = uncompress_asc_base64((const uint8*)des_data, siglen,(uint8 *) src_data, &src_len);
	if (result != 0)
	{
		free(src_data);
		free(des_data);
		return -4;
	}
	*outdata = src_data;
	*outdatalen = src_len;
	//out("原始数据长度为%d，解密后的数据长度%d\n", enc_file_s->main_len, *outdatalen);
	free(des_data);
	return 0;
}


int mb_dec_file_function(unsigned char *key_zlib_json, char *indata, int inlen, char **outdata, int *outdatalen)
{
	int result;
	int key_zlib_len=-1;
	int key_json_len=-1;
	char key_json[1024 * 1024] = {0};

	struct _key_sum key_sum;
	struct _enc_file_s enc_file_s;
	memset(&key_sum, 0, sizeof(struct _key_sum));
	memset(&enc_file_s, 0, sizeof(struct _enc_file_s));
	key_zlib_len = key_zlib_json[0] * 0x1000000 + key_zlib_json[1] * 0x10000 + key_zlib_json[2] * 0x100 + key_zlib_json[3] * 0x1;
	result = asc_uncompress((const unsigned char *)key_zlib_json + 10, key_zlib_len, (uint8 *)key_json, &key_json_len);
	if (result < 0 || key_json_len < 0)
	{
		out("密-钥文件解压失败\n");
		return -1;
	}
	//out("解压后的json key数据：%s\n", key_json);
	result = analysis_key_value(key_json, &key_sum);
	if (result < 0)
	{
		out("密-钥文件解析失败\n");
		return -2;
	}
	//out("解压后的json 数据文件：%s\n", indata);
	result = analysis_enc_file(indata, &enc_file_s);
	if (result < 0)
	{
		out("加-密文件解析失败\n");
		return -3;
	}

	//验证签名是否正确
	result = verify_file_sign(&key_sum, &enc_file_s);
	if (result < 0)
	{
		out("签-名验证失败\n");
		if (enc_file_s.main_body != NULL)
			free(enc_file_s.main_body);
		return -4;
	}

	//解密对称加密用的密钥串
	result = dec_key_value(&key_sum, &enc_file_s);
	if (result < 0)
	{
		out("解-密对称密钥串失败\n");
		if (enc_file_s.main_body != NULL)
			free(enc_file_s.main_body);
		return -5;
	}

	//解密主体内容
	result = dec_file_main_body(&enc_file_s, outdata, outdatalen);
	if (result < 0)
	{
		out("解-密主体内容失败\n");
		if (enc_file_s.main_body != NULL)
			free(enc_file_s.main_body);
		return -6;
	}

	if (enc_file_s.main_body != NULL)
		free(enc_file_s.main_body);
	return 0;
}

int mb_enc_singn_chect_function(unsigned char *key_zlib_json, char **outdata, int *outdatalen)
{
	int result;
	int key_zlib_len = 0;
	int key_json_len = 0;
	char key_json[1024 * 1024] = { 0 };

	struct _key_sum key_sum;
	struct _enc_file_s enc_file_s;
	memset(&key_sum, 0, sizeof(struct _key_sum));
	memset(&enc_file_s, 0, sizeof(struct _enc_file_s));

	key_zlib_len = key_zlib_json[0] * 0x1000000 + key_zlib_json[1] * 0x10000 + key_zlib_json[2] * 0x100 + key_zlib_json[3] * 0x1;
	result = asc_uncompress((const unsigned char *)key_zlib_json + 10, key_zlib_len, (uint8 *)key_json, &key_json_len);
	if (result < 0 || key_json_len < 0)
	{
		out("密-钥文件解压失败,result =%d,key_json_len = %d\n", result, key_zlib_len);
		return -1;
	}
	//out("解压后的json key数据：%s\n", key_json);
	result = analysis_key_value(key_json, &key_sum);
	if (result < 0)
	{
		out("密-钥文件解析失败\n");
		return -2;
	}

	int cout_num;

	//组包数据
	cJSON *json = cJSON_CreateObject();
	cJSON *svrpub_array,*terpiv_array;
	//cJSON *svrpub_item, *terpiv_item;
	char tmp_num[10];
	char check_data[100];
	char encdata_tmp[1024];
	char encdata[1024];
	char tersign_tmp[1024];
	char tersign[1024];
	int tersign_len;
	int encdata_len;
	cJSON_AddItemToObject(json, "svrpub", svrpub_array = cJSON_CreateArray());
	cJSON_AddItemToObject(json, "terpiv", terpiv_array = cJSON_CreateArray());
	for (cout_num = 0; cout_num < 5; cout_num++)
	{
		memset(tmp_num, 0, sizeof(tmp_num));
		memset(check_data, 0, sizeof(check_data));
		memset(encdata, 0, sizeof(encdata));
		memset(encdata_tmp, 0, sizeof(encdata_tmp));
		memset(tersign, 0, sizeof(tersign));
		memset(tersign_tmp, 0, sizeof(tersign_tmp));
		sprintf(tmp_num,"%d",cout_num+1);
		timer_read_asc(check_data);

		//公钥加密  public_encrypt(unsigned char*data, int data_len, unsigned char*key, unsigned char*encrypted)
		encdata_len = public_encrypt((uint8 *)check_data, strlen(check_data), (uint8 *)key_sum.public_key[cout_num].key_value, (uint8 *)encdata_tmp);
		EVP_EncodeBlock((uint8 *)encdata, (const uint8 *)encdata_tmp, encdata_len);

		cJSON *svrpub_layer;
		cJSON_AddItemToObject(svrpub_array, "dira", svrpub_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(svrpub_layer, "key_num_s", (const char *)tmp_num);
		cJSON_AddStringToObject(svrpub_layer, "checkdata", (const char *)check_data);
		cJSON_AddStringToObject(svrpub_layer, "encdata", (const char *)encdata);
		
		//私钥签名 private_sign(const unsigned char *in_str, unsigned int in_str_len, unsigned char *outret, unsigned int *outlen, unsigned char*key)
		MD5_CTX ctx;
		char outmd[20] = {0};
		MD5_Init(&ctx);
		MD5_Update(&ctx, check_data, strlen(check_data));
		MD5_Final((uint8 *)outmd, &ctx);
		private_sign((uint8 *)outmd, 16, (uint8 *)tersign_tmp, (unsigned int *)&tersign_len, (uint8 *)key_sum.private_key[cout_num].key_value);
		EVP_EncodeBlock((uint8 *)tersign, (const uint8 *)tersign_tmp, tersign_len);

		cJSON *terpiv_layer;
		cJSON_AddItemToObject(terpiv_array, "dira", terpiv_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(terpiv_layer, "key_num_t", (const char *)tmp_num);
		cJSON_AddStringToObject(terpiv_layer, "checkdata", (const char *)check_data);
		cJSON_AddStringToObject(terpiv_layer, "tersign", (const char *)tersign);

	}	
	*outdata = cJSON_Print(json);
	printf("json buf = %s\n", *outdata);
	*outdatalen = strlen(*outdata);
	printf("josn buf len = %d", *outdatalen);
	cJSON_Delete(json);
	return 0;
}


int mb_enc_get_mqtt_connect_info_json(uint8 *key_zlib_json, char *ter_id,int mqtt_type, char **json_data)
{
	int result;
	int key_json_len = -1;
	char key_json[1024 * 1024] = { 0 };
	struct _key_sum key_sum;
	struct _enc_file_s enc_file_s;
	memset(&key_sum, 0, sizeof(struct _key_sum));
	memset(&enc_file_s, 0, sizeof(struct _enc_file_s));



	char main_body[512] = { 0 };
	if (mqtt_type == 1)
		sprintf(main_body, "{\"id\": \"%s\",\"type\" : \"0\"}", ter_id);
	else
		sprintf(main_body, "{\"id\": \"%s\",\"type\" : \"9\"}", ter_id);
	//out("main_body = %s\n", main_body);

	enc_file_s.main_len = strlen(main_body);


	int key_zlib_len = key_zlib_json[0] * 0x1000000 + key_zlib_json[1] * 0x10000 + key_zlib_json[2] * 0x100 + key_zlib_json[3] * 0x1;
	result = asc_uncompress((const unsigned char *)key_zlib_json + 10, key_zlib_len, (uint8 *)key_json, &key_json_len);
	if (result < 0 || key_json_len < 0)
	{
		out("密-钥文件解压失败\n");
		return -1;
	}
	//out("解压后的json key数据：%s\n", key_json);
	result = analysis_key_value(key_json, &key_sum);
	if (result < 0)
	{
		out("密-钥文件解析失败\n");
		return -2;
	}

	char time_usec[100] = {0};
	timer_read_asc(time_usec);
	char key_num_str[3] = {0};
	char des_key_str[10] = {0};
	memcpy(key_num_str, time_usec+26,1);
	memcpy(des_key_str, time_usec + 21, 6);
	int key_num;
	
	key_num = atoi(key_num_str);
	if (key_num == 9)
		key_num = 5;
	else if(key_num == 8)
		key_num = 2;
	else if (key_num == 7)
		key_num = 3;
	else if (key_num == 6)
		key_num = 4;
	else if (key_num == 5)
		key_num = 5;
	else if (key_num == 4)
		key_num = 1;
	else if (key_num == 3)
		key_num = 2;
	else if (key_num == 2)
		key_num = 3;
	else if (key_num == 1)
		key_num = 4;
	else if (key_num == 0)
		key_num = 5;
	else
		key_num = 1;
	enc_file_s.key_num_s = key_num;
	enc_file_s.key_num_t = key_num;
	strncpy(enc_file_s.des_key, ter_id+2,10);
	strcpy(enc_file_s.des_key + strlen(enc_file_s.des_key), des_key_str);

	enc_key_value(&key_sum, &enc_file_s);

	enc_file_main_body(&enc_file_s, main_body);

	generate_file_sign(&key_sum, &enc_file_s);

	char tmp[1024];
	cJSON *json = cJSON_CreateObject();
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", enc_file_s.key_num_s);
	cJSON_AddStringToObject(json, "key_num_s", tmp);
	cJSON_AddStringToObject(json, "sign_string", enc_file_s.sign_string);
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", enc_file_s.key_num_t);
	cJSON_AddStringToObject(json, "key_num_t", tmp);
	cJSON_AddStringToObject(json, "enc_key", enc_file_s.enc_key);
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", enc_file_s.main_len);
	cJSON_AddStringToObject(json, "main_len", tmp);
	cJSON_AddStringToObject(json, "main_body", enc_file_s.main_body);
	cJSON_AddStringToObject(json, "id", ter_id);
	cJSON_AddStringToObject(json, "type", "0");
	if (mqtt_type == 1)
		cJSON_AddStringToObject(json, "mqtt_type", "0");
	else
		cJSON_AddStringToObject(json, "mqtt_type", "9");
	free(enc_file_s.main_body);
	
	char *g_buf;
	g_buf = cJSON_PrintUnformatted(json);
	//out("%s\n", g_buf);
	*json_data = g_buf;
	cJSON_Delete(json);
	return 0;
}