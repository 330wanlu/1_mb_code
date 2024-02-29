/*****************************************************************************
File name:   common.c
Description: ȫ�ֺ���������ȫ�ֱ���ת�����ַ�������
Author:      Zako
Version:     1.0
Date:        2022.1
History:
20220109     ��function.c����1000�д������ˣ���ʼ��
*****************************************************************************/
#include "character.h"

int EncordingConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf,
                     size_t outlen)
{
    int nOutBuffOrigLen = outlen;
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    int nRet = iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
    iconv_close(cd);
    if (nRet == -1)
        return -2;
    nRet = nOutBuffOrigLen - outlen;
    return nRet;
}

// utf8תgbk�����Ĭ�ϱ�����GBK��ת��
inline int U2G_GBKIgnore(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (!inlen)
        return 0;
#ifdef PROTOCOL_GBK
    if (outlen < inlen)
        return -1;
    memcpy(outbuf, inbuf, inlen);
    return inlen;
#else
    return EncordingConvert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
#endif
}

// utf8תgbk�����������UTF8��ת��
inline int U2G_UTF8Ignore(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (!inlen)
        return 0;
#ifdef PROTOCOL_GBK
    return EncordingConvert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
#else
    if (outlen < inlen)
        return -1;
    memcpy(outbuf, inbuf, inlen);
    return inlen;
#endif
}

// gbkתutf8�����Ĭ�ϱ�����GBK��ת��
inline int G2U_GBKIgnore(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (!inlen)
        return 0;
#ifdef PROTOCOL_GBK
    if (outlen < inlen)
        return -1;
    memcpy(outbuf, inbuf, inlen);
    return inlen;
#else
    return EncordingConvert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
#endif
}

// gbkתutf8�����������UTF8��ת��
inline int G2U_UTF8Ignore(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (!inlen)
        return 0;
#ifdef PROTOCOL_GBK
    return EncordingConvert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
#else
    if (outlen < inlen)
        return -1;
    memcpy(outbuf, inbuf, inlen);
    return inlen;
#endif
}

inline int UnicodeToDefaultEncording(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (!inlen)
        return 0;
    char szDefaultEnc[BS_8] = "";
#ifdef PROTOCOL_GBK
    strcpy(szDefaultEnc, "gbk");
#else
    strcpy(szDefaultEnc, "utf-8");
#endif
    return EncordingConvert("unicodelittle", szDefaultEnc, inbuf, inlen, outbuf, outlen);
}

//---------------------------------------Base64�ӽ����㷨��ʼ--------------------------------------
//����opensslԭ���ӽ��ܣ�Ч�ʸߣ�����ʱ����64�ֽڲ���ӻس�������ʱȥ��β�����00
//����ֵΪBase64֮����ַ�������
inline int Base64_Encode(const char *abTobeEncodedBuf, int nTobeEncodedDataLen,
                         char *szOutEncodeBuf)
{
    return EVP_EncodeBlock((uint8 *)szOutEncodeBuf, (const uint8 *)abTobeEncodedBuf,
                           nTobeEncodedDataLen);
}

inline int CalcDecodeLength(const char *b64input)
{
    int len = strlen(b64input);
    if (!len)
        return 0;
    int padding = 0;
    if (b64input[len - 1] == '=' && b64input[len - 2] == '=')  // last two chars are =
        padding = 2;
    else if (b64input[len - 1] == '=')  // last char is =
        padding = 1;
    return (int)len * 0.75 - padding;
}

//����ֵ��������: -1 error; >0 ��������ݳ���
inline int Base64_Decode(const char *szTobeDecodeB64Buf, int nTobeDecodeDataLen,
                         char *abOutDecodeBuf)
{
    int nRetLen = CalcDecodeLength(szTobeDecodeB64Buf);
    if (!nRetLen)
        return 0;
    if (EVP_DecodeBlock((uint8 *)abOutDecodeBuf, (const uint8 *)szTobeDecodeB64Buf,
                        nTobeDecodeDataLen) > 0)
        return nRetLen;
    else
        return -1;
}
//-------------------------------------Base64�ӽ����㷨����----------------------------------------
//����PROTOCOL_GBK�궨���в�ͬ��ת����ʽ,�����������Ҫ�㹻��һ��2������
int UTF8ToGBKBase64(char *szIn, int nInLen, char *szOut)
{
    if (!nInLen)
        return -1;
    int nMaxBuffLen = nInLen * 2 + 64;
    int nRet = -1;
    char *szGBK = (char *)calloc(1, nMaxBuffLen);
    while (1) {
        int nGBKLen = U2G_GBKIgnore(szIn, nInLen, szGBK, nMaxBuffLen);
        if (nGBKLen <= 0)  // less then szIn
            break;
        nRet = Base64_Encode(szGBK, nGBKLen, szOut);
        break;
    }
    free(szGBK);
    return nRet;
}

//����PROTOCOL_GBK�궨���в�ͬ��ת����ʽ,�п��ܳ���������С
int GBKBase64ToUTF8(char *szIn, char *szOut, int nOutBuffSize)
{
    int nTmpLen = strlen(szIn) * 2 + 256;
    char *pTmp = calloc(1, nTmpLen);
    if (!pTmp)
        return -1;
    int nRet = -2;
    while (1) {
        if (Base64_Decode(szIn, strlen(szIn), pTmp) < 0)
            break;
        //��ԭUTF8
        int nUTF8Len = G2U_GBKIgnore(pTmp, strlen(pTmp), szOut, nOutBuffSize);
        if (nUTF8Len <= 0)
            break;
        nRet = nUTF8Len;
        break;
    }
    free(pTmp);
    return nRet;
}

//���ڸ��������������Ӧ�������ж�xml���룬������ת����UTF8���㴦��
//<-1 unknow,0 gbk,1=utf8
int CheckXmlEncording(const char *szXml)
{
    //<?xml version="1.0" encoding="GBK"?>
    //<?xml version="1.0" encoding="UTF8"?>
    int nRet = -1;
    char szBuf[65] = "";
    char *pStart = strstr(szXml, "<?xml ");
    char *pEnd = strchr(szXml, '>');
    char *pInnerStart = NULL, *pInnerEnd = NULL;
    do {
        if (strlen(szXml) < 36)
            break;
        if (!pStart || !pEnd)
            break;
        int nSize = pEnd - pStart;
        if (nSize > 64 || nSize < 0)
            break;
        strncpy(szBuf, szXml, nSize + 1);
        pInnerStart = strstr(szBuf, "encoding=");
        if (!pInnerStart)
            break;
        pInnerStart += 10;
        if (!(pInnerEnd = strchr(pInnerStart, '"')))
            break;
        *pInnerEnd = '\0';
        if (!strcasecmp(pInnerStart, "gbk")) {
            nRet = 0;
            break;
        } else if (!strcasecmp(pInnerStart, "utf-8")) {
            nRet = 1;
            break;
        } else {
            _WriteLog(LL_WARN, "Unknow encording from xml");
        }
    } while (false);
    return nRet;
}

bool IsUTF8WithBom(char *pBytes, uint32 nSize)
{
    uint8 abUTF8HeadFlag[] = {0xef, 0xbb, 0xbf};
    int nUTF8FlagSize = sizeof(abUTF8HeadFlag);
    if (nSize > 3 && !memcmp(pBytes, abUTF8HeadFlag, nUTF8FlagSize))
        return true;  // ��bomͷUTF8�ֽ���
    return false;
}

//����ʲô���룬���ս�ascii�ַ�����ת���ɹ��̱���ĺ���
int AutoAsciiToChinese(char *pBytes, uint32 nSize, char *pOutBufBytes, uint32 nOutBufSize)
{
    if (IsUTF8WithBom(pBytes, nSize))  // ��bomͷUTF8�ֽ���
        return U2G_UTF8Ignore(pBytes + 3, nSize - 3, pOutBufBytes, nOutBufSize);
    else  // GBK�ֽ���
        return G2U_GBKIgnore((char *)pBytes, nSize, pOutBufBytes, nOutBufSize);
}

//����xmlͷ�������Զ�ת����ͷ��UTF8����ת����ͷ��GBK��ת��
int XmlBase64ToUTF8(char *szIn, char *szOut, int nOutBuffSize)
{
    int nTmpLen = strlen(szIn) * 2 + 256;
    char *pTmp = calloc(1, nTmpLen);
    int nUTF8Len = 0;
    if (!pTmp)
        return -1;
    int nRet = -2;
    while (1) {
        if (Base64_Decode(szIn, strlen(szIn), pTmp) < 0)
            break;
        // 20220628���ԣ�nisec˰�������з�����Ӧ��XML��UTF8��ʵ����GBK���룬ֻ��ͨ��Ӳ���������ͬ������xml�������ı�����ת��
        //���Խ����ҵ��Ϣͬ����������
        //  if (CheckXmlEncording(pTmp) == 0)
        if (IsUTF8(pTmp)) {
            snprintf(szOut, nOutBuffSize, "%s", pTmp);
            nUTF8Len = strlen(szOut);
        } else
            nUTF8Len = EncordingConvert("gbk", "utf-8", pTmp, strlen(pTmp), szOut, nOutBuffSize);
        if (nUTF8Len < 0) {
            _WriteLog(LL_WARN, "XmlBase64ToUTF8 failed");
            break;
        }
        nRet = nUTF8Len;
        break;
    }
    free(pTmp);
    return nRet;
}

//��GBK��UTF8�ĺ궨���޹أ�������ǿ��ת����ʽ
int GBKBase64ToUTF8_NP(char *szIn, char *szOut, int nOutBuffSize)
{
    int nTmpLen = strlen(szIn) * 2 + 256;
    char *pTmp = calloc(1, nTmpLen);
    if (!pTmp)
        return -1;
    int nRet = -2;
    while (1) {
        if (strchr(szIn, '\n')) {  // 76 �س����е�����Base64��ʽ����
            char *pNewBuf = (char *)calloc(1, strlen(szIn) + 1);
            if (!pNewBuf)
                break;
            strcpy(pNewBuf, szIn);
            char *pNewBuf2 = StrReplace(szIn, "\n", "");
            int nChildRet = Base64_Decode(pNewBuf2, strlen(pNewBuf2), pTmp);
            free(pNewBuf);
            free(pNewBuf2);
            if (nChildRet < 0)
                break;
        } else {
            if (Base64_Decode(szIn, strlen(szIn), pTmp) < 0)
                break;
        }
        int nUTF8Len = EncordingConvert("gbk", "utf-8", pTmp, strlen(pTmp), szOut, nOutBuffSize);
        if (nUTF8Len < 0) {
#ifdef PROTOCOL_GBK  //���ܳ��ַ�����Ϣ�м�����gbk�ְ���utf8���֣�iconv���޷�����
            _WriteLog(LL_INFO, "EncordingConvert failed nUTF8Len = %d", nUTF8Len);
            nUTF8Len = gbk2utf8((uint8 *)pTmp, (uint8 *)szOut);
            if (nUTF8Len < 0)
                break;
            nRet = nUTF8Len;
#endif
            break;
        }
        nRet = nUTF8Len;
        break;
    }
    free(pTmp);
    return nRet;
}


//���Ժ��ֲ��ּ�ⲻ��ȷ����������ʹ��
int IsGBK(const char *str)
{
    unsigned int nBytes = 0;  // GBK����1-2���ֽڱ���,��������,Ӣ��һ��
    uint8 chr = *str;
    int bAllAscii = 1;  //���ȫ������ASCII,
    int i = 0;
    for (i = 0; str[i] != '\0'; ++i) {
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0) {  // �ж��Ƿ�ASCII����,�������,˵���п�����GBK
            bAllAscii = 0;
        }
        if (nBytes == 0) {
            if (chr >= 0x80) {
                if (chr >= 0x81 && chr <= 0xFE) {
                    nBytes = +2;
                } else {
                    return 0;
                }
                nBytes--;
            }
        } else {
            if (chr < 0x40 || chr > 0xFE) {
                return 0;
            }
            nBytes--;
        }  // else end
    }
    if (nBytes != 0) {  //Υ������
        return 0;
    }
    if (bAllAscii) {  //���ȫ������ASCII,
                      //Ҳ��GBK
        return 1;
    }
    return 1;
}


bool IsUTF8(const char *string)
{
    if (!string)
        return true;
    const unsigned char *bytes = (const unsigned char *)string;
    unsigned int cp;
    int num;
    while (*bytes != 0x00) {
        if ((*bytes & 0x80) == 0x00) {
            // U+0000 to U+007F
            cp = (*bytes & 0x7F);
            num = 1;
        } else if ((*bytes & 0xE0) == 0xC0) {
            // U+0080 to U+07FF
            cp = (*bytes & 0x1F);
            num = 2;
        } else if ((*bytes & 0xF0) == 0xE0) {
            // U+0800 to U+FFFF
            cp = (*bytes & 0x0F);
            num = 3;
        } else if ((*bytes & 0xF8) == 0xF0) {
            // U+10000 to U+10FFFF
            cp = (*bytes & 0x07);
            num = 4;
        } else
            return false;
        bytes += 1;
        int i = 1;
        for (i = 1; i < num; ++i) {
            if ((*bytes & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (*bytes & 0x3F);
            bytes += 1;
        }
        if ((cp > 0x10FFFF) || ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
            ((cp <= 0x007F) && (num != 1)) || ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
            ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
            ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
            return false;
    }
    return true;
}

//++issue,˰�ֵ�UTF8���û���⣬�����ƺ������ҷ����뵼����json�������ڿ�Ʊ�ὫUTF8����GBK
int CheckStrEncoding(char *str)
{
    int result = !IsUTF8(str);
#ifdef PROTOCOL_GBK
    if (result != 1) {
        printf("Current environment is GBK, "
               "but import not a GBK file!\r\n");
        return -1;  //�����ʽ����
    }
#else
    if (result == 1) {
        printf("Current environment is UTF8, "
               "but import a GBK file!\r\n");
        return -1;  //�����ʽ����
    }
#endif
    return 0;
}


void mytrim(char *s, char c)
{
    char *t = s;
    while (*s == c) {
        s++;
    };
    if (*s) {
        char *t1 = s;
        while (*s) {
            s++;
        };
        s--;
        while (*s == c) {
            s--;
        };
        while (t1 <= s) {
            *(t++) = *(t1++);
        }
    }
    *t = 0;
}

//���Խ�����Сд��ĸת��Ϊ��д��ĸ
void mystrupr(char *str)
{
    char *curr = str;
    while (*curr != '\0') {
        //�б��Ƿ�Ϊ��ĸ isalpha () �б��Ƿ�ΪСд islower() ��Ϊctype.h�ṩ�ĺ���
        if (isalpha(*curr) && islower(*curr)) {
            *curr = toupper(*curr);
        }
        curr++;
    }
}

//�˺������Խ����д�д��ĸת��ΪСд��ĸ
void mystrlwr(char *str)
{
    char *curr = str;
    while (*curr != '\0') {
        // isupper() �ж���ĸ�Ƿ�Ϊ��д
        if (isalpha(*curr) && isupper(*curr)) {
            *curr = tolower(*curr);
        }
        curr++;
    }
}

//�˺������Խ��ַ���\�ֽ��� ���� mystrrev
void ByteReverse(void *pByte, int nInputSize)
{
    uint8 *pBegin = (uint8 *)pByte;
    uint8 *pEnd = (uint8 *)pByte + nInputSize - 1;
    uint8 pTemp;
    while (pBegin < pEnd) {
        pTemp = *pBegin;
        *pBegin = *pEnd;
        *pEnd = pTemp;
        ++pBegin, --pEnd;
    }
}

// 16��������ת�ַ��� 0x1234 �����ڴ�16����ת�ַ�����negatives�Ƿ���
void myxtoa(unsigned long val, char *buf, unsigned radix, int negative)
{
    char *p = NULL;
    char *firstdig;
    char temp;
    unsigned digval;
    p = buf;
    if (negative) {
        // Negative, so output '-' and negate
        *p++ = '-';
        val = (unsigned long)(-(long)val);
    }
    // Save pointer to first digit
    firstdig = p;
    do {
        digval = (unsigned)(val % radix);
        val /= radix;
        // Convert to ascii and store
        if (digval > 9) {
            *p++ = (char)(digval - 10 + 'a');
        } else {
            *p++ = (char)(digval + '0');
        }
    } while (val > 0);
    *p-- = '\0';
    do {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        p--;
        firstdig++;
    } while (firstdig < p);
}

//�ж��Ƿ�Ϊ����ת����binstr
//�ַ���ֻ����0-9, a-f���,1=ok, 0=failed
int CheckXDigit(char *str, int nCheckMax)
{
    int nOK = 1;
    char *curr = str;
    while (*curr != '\0' && nCheckMax-- > 0) {
        // isxdigit() �ж�0123456789abcdefABCDEF
        if (!isxdigit(*curr)) {
            nOK = 0;
            break;
        }
        curr++;
    }
    return nOK;
}

//------------------------------Str Byteת�� ������ begin------------
uint8 sub_100061B0(uint8 a1)
{
    uint8 result;
    if (a1 <= 9u)
        result = a1 + 48;
    else
        result = a1 + 87;
    return result;
}

uint8 sub_100061D0(uint8 a1)
{
    uint8 result;
    if (a1 <= 0x39u)
        result = a1 - 48;
    else
        result = a1 - 87;
    return result;
}

//����λ�ַ��������س��Ƚ��ᱻ�ض�
//"12345" -> 0x120x340x50 len=2;"1234"  -> 0x120x34 len=2;"1A05" -> 0x1a0x05 len=2;
//"1G23" -> 0 len=0; failed
// ��ת�����ַ���ֻ����0-9, a-f���
int Str2Byte(uint8 *szOut, char *szIn, int nInputLen)
{
    if (!CheckXDigit(szIn, nInputLen))
        return 0;
    char *szNewBuf = (char *)calloc(1, nInputLen + 1);
    memcpy(szNewBuf, szIn, nInputLen);
    mystrlwr(szNewBuf);  //ת��Сд��������
    signed int v3;
    int v4;
    int v5;
    int v6;
    uint8 *v7;
    uint8 v8;
    uint8 v9;
    v3 = nInputLen;
    v4 = nInputLen - ((unsigned __int64)nInputLen >> 32);
    v5 = 0;
    v6 = nInputLen / 2;
    if (nInputLen / 2 > 0) {
        v7 = (uint8 *)szNewBuf;
        do {
            v8 = sub_100061D0(*(_BYTE *)(v7 + 1));
            v9 = 16 * sub_100061D0(*(_BYTE *)v7) + v8;
            LOBYTE(v4) = (BYTE)*szOut;
            v7 += 2;
            *(_BYTE *)(v5++ + szOut) = v9;
        } while (v5 < v6);
        v3 = nInputLen;
    }
    if (v3 % 2 > 0) {
        LOBYTE(v4) = 16 * sub_100061D0(*(_BYTE *)(szNewBuf + v3 - 1));
        *(_BYTE *)(szOut + v6) = v4;
    }
    free(szNewBuf);
    return v5;
}

// 0x120x340x50->"123450" len=6
// 0x1a0x05->"1a05"  //ת������Сд�ַ���
int Byte2Str(char *szOut, uint8 *szIn, int nInputLen)
{
    uint8 *v5;
    int result = nInputLen;
    int v4 = 0;
    if (nInputLen > 0) {
        v5 = (uint8 *)szOut;
        do {
            *(_BYTE *)v5 = sub_100061B0(*(_BYTE *)(v4 + szIn) >> 4);
            *(_BYTE *)(v5 + 1) = sub_100061B0(*(_BYTE *)(v4 + szIn) & 0xF);
            result = nInputLen;
            ++v4;
            v5 += 2;
        } while (v4 < nInputLen);
    }
    return result * 2;  // 0x010x02 -> 0102
}

uint8 HexChar(char c)
{
    if ('0' <= c && c <= '9')
        return (uint8)(c - '0');
    if ('A' <= c && c <= 'F')
        return (uint8)(c - 'A' + 10);
    if ('a' <= c && c <= 'f')
        return (uint8)(c - 'a' + 10);
    return (uint8)(-1);
}

//ֱ��ԭʼת������isxdigit�ȼ��
int Str2ByteRaw(const char *s, uint8 *buff)
{
    int result = 0;
    if (!s || !buff)
        return 0;

    while (*s) {
        uint8 nib1 = HexChar(*s++);
        if ((signed)nib1 < 0)
            return 0;
        uint8 nib2 = HexChar(*s++);
        if ((signed)nib2 < 0)
            return 0;

        uint8 bin = (nib1 << 4) + nib2;
        *buff++ = bin;
        ++result;
    }
    return result;
}

//ֱ��ԭʼת������isxdigit�ȼ��,output������length*2+16��������
//ת����ΪСд�ַ���
int Byte2StrRaw(const uint8 *buff, int length, char *output)
{
    char *porigOutbuf = output;
    char binHex[] = "0123456789abcdef";
    if (!output)
        return 0;

    *output = '\0';
    if (!buff || length <= 0)
        return 0;

    for (; length > 0; --length) {
        uint8 byte = *buff++;
        *output++ = binHex[(byte >> 4) & 0x0F];
        *output++ = binHex[byte & 0x0F];
    }
    return output - porigOutbuf;
}

//------------------------------Str Byteת�� ������ finish------------
char *StrReplace(char *src, char *from, char *to)
{
    int size = strlen(src) + 1;
    int fromlen = strlen(from);
    int tolen = strlen(to);
    char *value = malloc(size);
    char *dst = value;
    if (value != NULL) {
        for (;;) {
            const char *match = strstr(src, from);
            if (match != NULL) {
                int count = match - src;
                char *temp;
                size += tolen - fromlen;
                temp = realloc(value, size);
                if (temp == NULL) {
                    free(value);
                    return NULL;
                }
                dst = temp + (dst - value);
                value = temp;
                memmove(dst, src, count);
                src += count;
                dst += count;
                memmove(dst, to, tolen);
                src += fromlen;
                dst += tolen;
            } else {
                strcpy(dst, src);
                break;
            }
        }
    }
    return value;
}

uint8 *MyMemSearch(uint8 *pBegin, uint8 *pEnd, uint8 *pBytes, int nsize)
{
    int count;
    uint8 *pDst;

    while ((long)pBegin + nsize <= (long)pEnd) {
        pDst = pBytes;
        count = 0;
        while (count < nsize && *pBegin == *pDst) {
            pBegin++;
            pDst++;
            count++;
        }
        if (count == nsize)
            break;
        pBegin = pBegin - count + 1;
    }
    if (count == nsize) {
        return (uint8 *)((long)pBegin - (long)count);
    } else {
        return NULL;
    }
}

//---------------------�ַ����ָʼ---------------------
//�ú�������δ֪�����ʽ����󳤶ȵ�������ɷ�ֹ������׽ڵ�Ϊջ���䣬���Ǻ����ڵ��Ƕѷ��䣬
//�ǵõ���free���գ�ʹ��ǰ����0��memset(&si, 0,sizeof(SplitInfo));
//����str1|str2||������������һ��ķָ�����޷������
// delimֻ���Ǽ��ַ������Ǹ����ַ���������\r\n�������ֱ����и������ͬʱ
void GetSplitString(char *str, char *delim, SplitInfo *splitInfo)
{
    int n = 0;
    char *next = NULL;
    SplitInfo *top = splitInfo;
    if (!strstr(str, delim))
        return;
    char *arr = calloc(1, strlen(str) + 1);
    if (!arr)
        return;
    strcpy(arr, str);
    next = strtok(arr, delim);
    if (next == NULL) {
        splitInfo->num = 0;
        splitInfo->next = NULL;
        goto GetSplitString_Finish;
    }
    SplitInfo *in = (SplitInfo *)malloc(sizeof(SplitInfo));
    memset(in, 0, sizeof(SplitInfo));
    in->info = calloc(1, strlen(next) + 1);
    strcpy(in->info, next);
    in->next = NULL;
    top->next = in;
    top = in;
    n++;
    while ((next = strtok(NULL, delim)) != NULL) {
        if (!strlen(next))
            continue;
        SplitInfo *te = (SplitInfo *)malloc(sizeof(SplitInfo));
        te->info = calloc(1, strlen(next) + 1);
        strcpy(te->info, next);
        te->next = NULL;
        top->next = te;
        top = te;
        n++;
    }
    splitInfo->num = n;
GetSplitString_Finish:
    free(arr);
}

SplitInfo *GetSplitIndex(SplitInfo *spRoot, uint16 wIndex)
{
    SplitInfo *pstNeed = NULL;
    SplitInfo *pOut = spRoot;
    int i = 0;
    for (i = 0; i < spRoot->num; i++) {
        pOut = pOut->next;
        if (i < wIndex)
            continue;
        pstNeed = pOut;
        break;
    }
    return pstNeed;
}

void SplitFree(SplitInfo *splitInfo)
{
    //�׽ڵ�infoΪNULL
    SplitInfo *top = splitInfo->next;
    while (top) {
        SplitInfo *next = top->next;
        if (top->info)
            free(top->info);
        free(top);
        top = next;
    }
    splitInfo->next = NULL;
    splitInfo->num = 0;
}

// ��str�ַ���spl�ָ�,����dst�У����������ַ�������;dst����Ϊ dst[X][1024]��ʽ
//�ú���������֪�����ʽ����󳤶ȵ����,ע��ú����и�����в��ܰ�����ֵ����"|1||3|"ֻ���и��2������
int GetSplitStringSimple(char *szInput, const char *spl, char dst[][768], int nMaxSplit)
{
    char szTmpBuf[1024] = "";
    if (!nMaxSplit)
        return -1;
    if (strlen(szInput) > sizeof(szTmpBuf) - 1)
        return -2;
    strcpy(szTmpBuf, szInput);
    int n = 0;
    char *result = NULL;
    result = strtok(szTmpBuf, spl);
    while (result != NULL) {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
        if (n == nMaxSplit)
            break;
    }
    return n;
}

// �ڳ��ַ����и��ݸ������ַ����ָ��ַ�����
// ��Ҫ���ⲿ����ָ��ָ����������ݣ���ȡ���ݺ����ⲿfree��
// input	�����볤��
// splitstr�������Ӵ�
// output	������������
// num		���Ӵ�����
// ��Ҫ�ֶ��ͷ� for (i = 0; i < num; i++) {free(apbyRtn[i]);}
int GetSplitSubString(char *input, char *splitstr, char **output, int *num)
{
    int tmplen = 0;
    int totallen = 0;
    int count = 0;
    // int nRtn = 0;
    char *p = input;
    int nSrcLen = strlen(input);
    int nSplStrLen = strlen(splitstr);
    if (input == NULL || nSrcLen == 0 || splitstr == NULL || nSplStrLen == 0)
        return -1;
    //����һ���ַ���̴���ͬ�����̴����ȱȽ�
    while ((*p != '\0')) {
        if (*p == splitstr[0]) {
            if (0 == strncmp(p, splitstr, nSplStrLen)) {
                output[count] = (char *)malloc(tmplen + 1);
                strncpy(output[count], input + totallen, tmplen);
                (output[count])[tmplen] = '\0';
                count++;
                totallen += (tmplen + nSplStrLen);
                p += nSplStrLen;
                tmplen = 0;
                continue;
            }
        }
        p++;
        tmplen++;
    }
    output[count] = (char *)malloc(tmplen + 1);
    strncpy(output[count], input + totallen, tmplen);
    (output[count])[tmplen] = '\0';
    count++;
    *num = count;
    return 0;
}
//---------------------�ַ����ָ����---------------------

//��UTF8������GBK���壬nFillSpaceLen!=0��ʹ��0x20���GBK������
int UTF8CopyGBK(uint8 *pGBKBuff, uint8 *pUTF8Buff, int nUTF8Len, int nFillSpaceLen)
{
    if (nFillSpaceLen != 0)
        memset(pGBKBuff, 0x20, nFillSpaceLen);
    if (nUTF8Len <= 0)
        return -1;
    size_t nNewBuffLen = nUTF8Len * 2 + 256;
    int nRet = -1;
    uint8 *pNewBuff = (uint8 *)calloc(1, nNewBuffLen);
    if (pNewBuff == NULL)
        return -2;
    while (1) {
#ifdef PROTOCOL_GBK
        memcpy(pGBKBuff, pUTF8Buff, nUTF8Len);  // mqtt�ӿ����ݼ�gbk������ת��,�˴���������
        nRet = nUTF8Len;
#else
        int nChildRet = U2G_GBKIgnore((char *)pUTF8Buff, nUTF8Len, (char *)pNewBuff, nNewBuffLen);
        if (nChildRet <= 0)
            break;
        memcpy(pGBKBuff, pNewBuff, nChildRet);
        nRet = nChildRet;
#endif
        break;
    }
    free(pNewBuff);
    return nRet;
}

// szInOutBuf�������ᱻ�Ķ�����΢������˴���Ļ�������Ҫ�㹻��С
int MemoryFormart(uint8 bOPType, char *szInOutBuf, int nInBufLen)
{
    int i, nRet = -1;
    if (nInBufLen == 0)
        return 0;
    switch (bOPType) {
        case MF_REMOVE_HEADZERO: {  // string:remove head 0; 00000123->123
            int nLen = strlen(szInOutBuf);
            ByteReverse(szInOutBuf, nLen);
            for (i = nLen - 1; i > 0; i--) {
                if ('0' == szInOutBuf[i])
                    szInOutBuf[i] = '\0';
                else
                    break;
            }
            nLen = strlen(szInOutBuf);
            ByteReverse((char *)szInOutBuf, nLen);
            nRet = nLen;
            break;
        }
        case MF_TRANSLATE_SPACE2ZERO: {  //  0x20 -> 0x00
            for (i = 0; i < nInBufLen; i++) {
                if (0x20 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x00;
                }
            }
            //ȷʵ����Ҫ�ض�
            nRet = strlen(szInOutBuf);
            break;
        }
        case MF_TRANSLATE_ZERO2SPACE: {  // array:0x00 -> 0x20
            for (i = 0; i < nInBufLen; i++) {
                if (0x00 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x20;
                }
            }
            nRet = strlen(szInOutBuf);
            break;
        }
        case MF_TRANSLATE_ABSOLUTE: {  //�ַ�����ʽ���ȡ����ֵ
            if ('-' != szInOutBuf[0])
                break;
            memmove(szInOutBuf, szInOutBuf + 1, nInBufLen - 1);
            szInOutBuf[nInBufLen - 1] = '\0';
            break;
        }
        case MF_TRANSLATE_RED: {  //����ֵ�ַ�����ʽ���ȡ��Ʊ���
            if ('-' == szInOutBuf[0])
                break;
            if (0 == atof(szInOutBuf))  // 0.00���߷�������ת���ַ� ��ת��
                break;
            memmove(szInOutBuf + 1, szInOutBuf, nInBufLen);
            szInOutBuf[0] = '-';
            break;
        }
        case MF_TRANSLATE_HEADTAILZERO: {  // " abc "->"abc"
            mytrim(szInOutBuf, ' ');
            break;
        }
        case MF_TRANSLATE_TAILZERO: {  // string:remove tail 0; 0.123000->0.123
            int nLen = strlen(szInOutBuf);
            if (!strstr(szInOutBuf, "."))
                return 0;
            for (i = nLen - 1; i > 0; i--) {
                if ('0' == szInOutBuf[i])
                    szInOutBuf[i] = '\0';
                else
                    break;
            }
            nRet = nLen;
            break;
        }
        case MF_TRANSLATE_SPACE2STRZERO: {  //" abc "->"0abc0"
            for (i = 0; i < nInBufLen; i++) {
                if (0x20 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x30;
                }
            }
            break;
        }
        case MF_REMOVE_TAILCOMMMA: {  //"ab,cd,"->"ab,cd"
            char *p = szInOutBuf + strlen(szInOutBuf) - 1;
            if (*p == ',')
                *p = '\0';
            break;
        }
        default:
            break;
    }
    return nRet;
}

bool CheckHeadTailSpace(char *szStr)
{
    return szStr[0] == 0x20 || szStr[strlen(szStr) - 1] == 0x20;
}

// old:str_cls_character
void StringRemoveChar(char *buf, uint32 len, char del_char)
{
    uint32 i, j;
    for (i = 0, j = 0; i < len; i++) {
        if (buf[i] == del_char)
            continue;
        buf[j++] = buf[i];
    }
    memset(buf + j, 0, len - j);
}

// nisec cntax ��չ��Ϣ
uint8 GetUint8FromString(char *szStringTwoBytes)
{
    uint8 ret = 0;
    Str2Byte(&ret, szStringTwoBytes, 2);
    return ret;
}

// aisino��չ��Ϣ
uint8 GetOneByteFromString(char *szStringOneBytes)
{
    char szBuf[4] = "";
    strncpy(szBuf, szStringOneBytes, 1);
    return (uint8)atoi(szBuf);
}

// szStr������ջ���ַ������������ַ�����ʽ��Ʊ��������ۼӣ�֧���ַ���ǰ����0
bool StringNumIncrement(char *szStr)
{
    char szBuf[32] = "";
    int nLen = 0;
    bool ret = false;
    do {
        if (!szStr)
            break;
        nLen = strlen(szStr);
        if (!nLen || nLen > 32)
            break;
        //�ۼ���Ҫ����ַ���ǰ���Ƿ���"0"
        long long lNum = atoll(szStr);
        if (!lNum)
            break;
        lNum++;
        sprintf(szBuf, "%lld", lNum);
        memset(szStr, 0x30, nLen);
        int nNewLen = strlen(szBuf);
        memcpy(szStr + nLen - nNewLen, szBuf, nLen);
        ret = true;
    } while (false);
    return ret;
}

int FindStringFromEvBuffer(struct evbuffer *buf, const char *szFlagBegin, const char *szFlagStop,
                           char *szOut, int nOutBufSize)
{
    int nResultSize = 0;
    struct evbuffer_ptr pStart = evbuffer_search(buf, szFlagBegin, strlen(szFlagBegin), NULL);
    if (pStart.pos == -1)
        return ERR_BUF_CHECK;
    evbuffer_ptr_set(buf, &pStart, strlen(szFlagBegin), EVBUFFER_PTR_ADD);
    struct evbuffer_ptr pEnd = evbuffer_search(buf, szFlagStop, strlen(szFlagStop), &pStart);
    nResultSize = pEnd.pos - pStart.pos;
    if (pEnd.pos == -1 || nResultSize < 0 || nResultSize > nOutBufSize)
        return ERR_BUF_CHECK;
    evbuffer_copyout_from(buf, &pStart, szOut, nResultSize);
    szOut[nResultSize] = '\0';
    return RET_SUCCESS;
}