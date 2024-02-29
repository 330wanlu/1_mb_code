#ifndef COMMON_CHARACTER_H
#define COMMON_CHARACTER_H
#include "common.h"

//非全局，一些函数级别枚举定义，跟在函数声明头文件
enum enMemoryFormat {
    MF_REMOVE_HEADZERO,          // 移除字符串前置0; 00000123->123,trimstart
    MF_TRANSLATE_SPACE2ZERO,     // array:0x20 -> 0x00
    MF_TRANSLATE_ZERO2SPACE,     // array:0x00 -> 0x20
    MF_TRANSLATE_ABSOLUTE,       //字符串格式金额取绝对值
    MF_TRANSLATE_RED,            //字符串格式金额取红票取负值
    MF_TRANSLATE_HEADTAILZERO,   //移除字符串首尾0，00123000->123,trim
    MF_TRANSLATE_TAILZERO,       //移除字符串尾0，0.123000->0.123,trim
    MF_TRANSLATE_SPACE2STRZERO,  // array:0x20 -> '0'(0x30)
    MF_REMOVE_TAILCOMMMA,        //移除末尾逗号
};

typedef struct split {
    char *info;
    int num;
    struct split *next;
} SplitInfo;

int EncordingConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf,
                     size_t outlen);
int U2G_GBKIgnore(char *inbuf, int inlen, char *outbuf, int outlen);
int G2U_GBKIgnore(char *inbuf, int inlen, char *outbuf, int outlen);
int U2G_UTF8Ignore(char *inbuf, int inlen, char *outbuf, int outlen);
int G2U_UTF8Ignore(char *inbuf, int inlen, char *outbuf, int outlen);
int Base64_Encode(const char *abTobeEncodedBuf, int nTobeEncodedDataLen, char *szOutEncodeBuf);
int Base64_Decode(const char *szTobeDecodeB64Buf, int nTobeDecodeDataLen, char *abOutDecodeBuf);
int UTF8ToGBKBase64(char *szIn, int nInLen, char *szOut);
int GBKBase64ToUTF8(char *szIn, char *szOut, int nOutBuffSize);
int GBKBase64ToUTF8_NP(char *szIn, char *szOut, int nOutBuffSize);
int XmlBase64ToUTF8(char *szIn, char *szOut, int nOutBuffSize);
int IsGBK(const char *str);
bool IsUTF8(const char *string);
int CheckStrEncoding(char *str);
int Str2Byte(uint8 *szOut, char *szIn, int nInputLen);
int Byte2Str(char *szOut, uint8 *szIn, int nInputLen);
char *StrReplace(char *src, char *from, char *to);
uint8 *MyMemSearch(uint8 *pBegin, uint8 *pEnd, uint8 *pBytes, int nsize);
void GetSplitString(char *str, char *delim, SplitInfo *splitInfo);
SplitInfo *GetSplitIndex(SplitInfo *spRoot, uint16 wIndex);
void SplitFree(SplitInfo *splitInfo);
int GetSplitStringSimple(char *szInput, const char *spl, char dst[][768], int nMaxSplit);
int GetSplitSubString(char *input, char *splitstr, char **output, int *num);
void mystrupr(char *str);
void mystrlwr(char *str);
void ByteReverse(void *pByte, int nInputSize);
void mytrim(char *s, char c);
void myxtoa(unsigned long val, char *buf, unsigned radix, int negative);
int CheckXDigit(char *str, int nCheckMax);
int UTF8CopyGBK(uint8 *pGBKBuff, uint8 *pUTF8Buff, int nUTF8Len, int nFillSpaceLen);
int MemoryFormart(uint8 bOPType, char *szInOutBuf, int nInBufLen);
bool CheckHeadTailSpace(char *szStr);
int Str2ByteRaw(const char *s, uint8 *buff);
int Byte2StrRaw(const uint8 *buff, int length, char *output);
void StringRemoveChar(char *buf, uint32 len, char del_char);
uint8 GetUint8FromString(char *szStringTwoBytes);
uint8 GetOneByteFromString(char *szStringOneBytes);
int AutoAsciiToChinese(char *pBytes, uint32 nSize, char *pOutBufBytes, uint32 nOutBufSize);
bool IsUTF8WithBom(char *pBytes, uint32 nSize);
bool StringNumIncrement(char *szStr);
int FindStringFromEvBuffer(struct evbuffer *buf, const char *szFlagBegin, const char *szFlagStop,
                           char *szOut, int nOutBufSize);
inline int UnicodeToDefaultEncording(char *inbuf, int inlen, char *outbuf, int outlen);
#endif