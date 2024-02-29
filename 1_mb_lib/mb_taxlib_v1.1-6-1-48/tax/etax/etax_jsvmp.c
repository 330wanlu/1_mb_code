/*****************************************************************************
File name:   etax_jsvmp.c
Description: 用于上海税务局全电功能中涉及JS指令虚拟化引擎的C实现
Author:      Zako
Version:     1.0
Date:        2023.04
History:
20230401     代码分流和最初代码实现
*****************************************************************************/
#include "etax_confirm.h"
//---------数字证书服务器算法 Jsvmp web指令虚拟化引擎 C语言实现部分算法功能mini版-------------------

enum enDpptJsvmpCode {
    DPPT_JSVMPCODE_VAR,
    DPPT_JSVMPCODE_CLS,
    DPPT_JSVMPCODE_MOV,
    DPPT_JSVMPCODE_ADD,
    DPPT_JSVMPCODE_SUB,
    DPPT_JSVMPCODE_MUL,
    DPPT_JSVMPCODE_DIV,
    DPPT_JSVMPCODE_MOD,
    DPPT_JSVMPCODE_EXP,
    DPPT_JSVMPCODE_INC,
    DPPT_JSVMPCODE_DEC,
    DPPT_JSVMPCODE_LT,
    DPPT_JSVMPCODE_GT,
    DPPT_JSVMPCODE_EQ,
    DPPT_JSVMPCODE_LE,
    DPPT_JSVMPCODE_GE,
    DPPT_JSVMPCODE_NE,
    DPPT_JSVMPCODE_WEQ,
    DPPT_JSVMPCODE_WNE,
    DPPT_JSVMPCODE_LG_AND,
    DPPT_JSVMPCODE_LG_OR,
    DPPT_JSVMPCODE_AND,
    DPPT_JSVMPCODE_OR,
    DPPT_JSVMPCODE_XOR,
    DPPT_JSVMPCODE_SHL,
    DPPT_JSVMPCODE_SHR,
    DPPT_JSVMPCODE_ZSHR,
    DPPT_JSVMPCODE_JMP,
    DPPT_JSVMPCODE_JE,
    DPPT_JSVMPCODE_JNE,
    DPPT_JSVMPCODE_JG,
    DPPT_JSVMPCODE_JL,
    DPPT_JSVMPCODE_JIF,
    DPPT_JSVMPCODE_JF,
    DPPT_JSVMPCODE_JGE,
    DPPT_JSVMPCODE_JLE,
    DPPT_JSVMPCODE_PUSH,
    DPPT_JSVMPCODE_POP,
    DPPT_JSVMPCODE_CALL,
    DPPT_JSVMPCODE_PRINT,
    DPPT_JSVMPCODE_RET,
    DPPT_JSVMPCODE_PAUSE,
    DPPT_JSVMPCODE_EXIT,
    DPPT_JSVMPCODE_CALL_CTX,
    DPPT_JSVMPCODE_CALL_VAR,
    DPPT_JSVMPCODE_CALL_REG,
    DPPT_JSVMPCODE_MOV_CTX,
    DPPT_JSVMPCODE_MOV_PROP,
    DPPT_JSVMPCODE_SET_CTX,
    DPPT_JSVMPCODE_NEW_OBJ,
    DPPT_JSVMPCODE_NEW_ARR,
    DPPT_JSVMPCODE_NEW_REG,
    DPPT_JSVMPCODE_SET_KEY,
    DPPT_JSVMPCODE_FUNC,
    DPPT_JSVMPCODE_ALLOC,
    DPPT_JSVMPCODE_PLUS,
    DPPT_JSVMPCODE_MINUS,
    DPPT_JSVMPCODE_NOT,
    DPPT_JSVMPCODE_VOID,
    DPPT_JSVMPCODE_DEL,
    DPPT_JSVMPCODE_NEG,
    DPPT_JSVMPCODE_TYPE_OF,
    DPPT_JSVMPCODE_IN,
    DPPT_JSVMPCODE_INST_OF,
    DPPT_JSVMPCODE_MOV_THIS,
    DPPT_JSVMPCODE_TRY,
    DPPT_JSVMPCODE_TRY_END,
    DPPT_JSVMPCODE_THROW,
    DPPT_JSVMPCODE_GET_ERR,
    DPPT_JSVMPCODE_MOV_ARGS,
    DPPT_JSVMPCODE_FORIN,
    DPPT_JSVMPCODE_FORIN_END,
    DPPT_JSVMPCODE_BREAK_FORIN,
    DPPT_JSVMPCODE_CONT_FORIN,
    DPPT_JSVMPCODE_BVAR,
    DPPT_JSVMPCODE_BLOCK,
    DPPT_JSVMPCODE_END_BLOCK,
    DPPT_JSVMPCODE_CLR_BLOCK,
};

struct EtaxJsvmpHeapStackItem {
    bool bStringOrNum;
    double num;
    struct evbuffer *string;  //部分字符串参数运算比较耗费内存,而且算法拼接此处不固定，只能动态内存
};
struct EtaxJsvmpStack {
    int fp;                    //兼容税局业务算法
    struct evkeyvalq kvStack;  // stack栈
    struct evkeyvalq kvHeap;   // heap堆(官方为object属性)
    //注意:stack0保留(用于0x70返回) stack1保留（用户保存0x50 raw string返回）

    //额外的一些必要参数
    uint8 *abWebshellCode;
    struct evkeyvalq *kvStringTableClear;
    struct evkeyvalq *kvStringTableAll;
    int nStringTableSize;
    int nStringTablesIndex;
    int nMidNumer;
    int nRolFinishCheckNum;
    bool bMatchFlag;
};
typedef struct EtaxJsvmpStack *JsStack;

bool FormatGetStringsTable(uint8 *pBuf, int nBufSize, struct evkeyvalq *kvs)
{
    int nIndex = 0, nNumber = 0;
    uint32 unSize = 0;
    uint8 *pData = NULL;
    bool bRet = true;
    char szBuf[BS_BLOCK], szNumBuf[BS_8];
    do {
        unSize = *(uint32 *)(pBuf + nIndex);
        pData = pBuf + nIndex + 4;
        if (unSize > sizeof(szBuf)) {
            bRet = false;
            break;
        }
        // unSize == 0情况已兼容
        memset(szBuf, 0, sizeof(szBuf));
        if (UnicodeToDefaultEncording((char *)pData, unSize * 2, szBuf, sizeof(szBuf)) < 0) {
            bRet = false;
            break;
        }
        sprintf(szNumBuf, "%d", nNumber);
        evhttp_add_header(kvs, szNumBuf, szBuf);

        nIndex += 4 + unSize * 2;
        nNumber++;
    } while (nIndex < nBufSize);
    return bRet;
}

//没用到的暂且不管,<0报错(部分数值待修正)
int GetDataTypeDefineSize(uint8 bVarTypeCode, int nDefaultValue)
{
    //部分返回值因为技术原因无法返回一致值，只能返回nRet=0
    int nRet = -1;
    switch (bVarTypeCode) {
        case 0x10:  //从object this.heap堆上获取数据
            nRet = nDefaultValue;
            break;
        case 0x0:
            //[差异，无法调整!]此处返回this.fp对应值
            if (nDefaultValue <= -4) {
                // this.stack[this.fp-bDefaultValue]
            } else {
                // this.stack[this.fp+bDefaultValue]
            }
            nRet = 1;  //支持该值
            break;
        case 0x60:
        case 0x30:
        case 0x80:  //返回stringstables函数，我们改为序号
            nRet = 1;
            break;
        case 0x20:  //读取栈上指定bDefaultValue数值
            break;
        case 0x50:  //原函数此处返回原始字符串表(二进制解出来的那个)中指定索引的字符串
            nRet = nDefaultValue;
            break;
        case 0x40:  //返回functiontables函数，我们改为序号
            nRet = 1;
            break;
        case 0x70:  //[差异，无法调整!]返回this.stack[0]栈底数值(我们代码没有栈，因此返回0)
            nRet = 1;
            break;
        case 0x90:  //返回bool值
            nRet = 1;
            break;
        case 0xa0:
            break;
        case 0xb0:
            break;
        case 0xd0:  //返回object的属性值get，等同于局部变量，变量存活周期很短
            nRet = 1;
            break;
        default:
            _WriteLog(LL_FATAL, "Unknow data type");
            break;
    }
    return nRet;
}

int DataSizeZeroAndDateTypeNotEqual0x30(uint8 *EipBufHead, uint8 bDataSize)
{
    double d = 0;
    int nVar = 8 - bDataSize, i = 0;
    uint32 nRet = 0;
    uint8 *p = (uint8 *)&d;
    for (i = 0; i < bDataSize; i++) {
        p[nVar + i] = *(EipBufHead + i);
    }
    nRet = (int)d;
    return nRet;
}

// this['\x6e\x65\x78\x74\x4f\x70\x65\x72\x61\x6e' + '\x74']()
//第一个参数输入，后三个输出，返回值为累加的eip数值
int GetCommandTypeSizeIndex(uint8 *pEipBegin, int nCodeEip, uint8 *pOutDataType, int *pOutDataValue,
                            uint8 *pbDataSize)
{
    int nCodeAddSize = 0, nChildRet = 0, nT = 0;
    uint8 *EipBufHead = pEipBegin + nCodeEip;
    do {
        *pOutDataType = *(EipBufHead)&240;
        *pbDataSize = *(EipBufHead) & (-241);
        if (0x30 == *pOutDataType) {
            *pOutDataValue = DataSizeZeroAndDateTypeNotEqual0x30(EipBufHead + 1, *pbDataSize);
        } else {
            if (0 == *pbDataSize)
                *pOutDataValue = 0;
            else if (1 == *pbDataSize)
                *pOutDataValue = *(EipBufHead + 1);
            else if (2 == *pbDataSize)
                *pOutDataValue = *(int16 *)(EipBufHead + 1);
            else if (4 == *pbDataSize)
                *pOutDataValue = *(int32 *)(EipBufHead + 1);
            else {
                nCodeAddSize = ERR_GENERIC;
                break;  //不识别的存储类型
            }
        }
        nCodeAddSize += *pbDataSize;
        nChildRet = GetDataTypeDefineSize(*pOutDataType, *pOutDataValue);
        if (nChildRet < 0) {
            nCodeAddSize = ERR_GENERIC;
            break;  //不识别的存储类型
        } else if (nChildRet == 0)
            *pOutDataValue = 0;  //无法支持的数据类型，返回值无意义，因此置0返回值
        nCodeAddSize += 1;       //默认指令进一位
    } while (false);
    nCodeAddSize > 0 ? (nCodeAddSize = nCodeEip + nCodeAddSize) : (nCodeAddSize = ERR_GENERIC);
    return nCodeAddSize;
}

// bMethod == 0, init;==1 free and clear jsStack; ==2 free item
int HeapStackMethod(JsStack jsStack, uint8 bMethod, char *itemString)
{
    if (0 == bMethod) {
        memset(jsStack, 0, sizeof(struct EtaxJsvmpStack));
        TAILQ_INIT(&jsStack->kvStack);
        TAILQ_INIT(&jsStack->kvHeap);
    } else if (1 == bMethod) {
        struct evkeyval *header;
        TAILQ_FOREACH(header, &jsStack->kvStack, next)
        {
            HeapStackMethod(jsStack, 2, header->value);
        }
        TAILQ_FOREACH(header, &jsStack->kvHeap, next)
        {
            HeapStackMethod(jsStack, 2, header->value);
        }
        evhttp_clear_headers(&jsStack->kvStack);
        evhttp_clear_headers(&jsStack->kvHeap);
    } else if (2 == bMethod) {
        struct EtaxJsvmpHeapStackItem *pItem = NULL;
        sscanf(itemString, "%p", &pItem);
        evbuffer_free(pItem->string);
        free(pItem);
    }
    return RET_SUCCESS;
}

//注意，需要先找到总的object处理函数其中堆栈中能明显看到set,get方法名称，该object函数根据类型和索引存储和读取函数，可以方便下断点
//直接在chunk[random].js中查找class,9个匹配值的第二个一般就是object对象处理函数
int RestoreStringsTable(uint8 *abWebShellCode, int nWebShellCodeSize,
                        struct evkeyvalq *kvStringTableAll, struct evkeyvalq *kvStringTableClear,
                        int *pnStringTableSize)
{
    uint8 *pEipBegin = NULL, *pEipBeginNext = NULL, bOpCode = 0, bOutDataType = 0, bOutDataSize = 0,
          abFlag[] = "\x34\x11";
    int nRet = ERR_GENERIC, nCodeEip = 0, nChildRet = 1, nOutDataValue = 0, nClearIndex = 0;
    char szBufNum[BS_8] = "", *pValue = NULL;
    do {
        //前512字节一般没有,341111,34110b都存在
        // 00000000: 0b32 2440 510e|3411 0b32 2640 510f 3411  .2$@Q.4..2&@Q.4.
        // 00000010: 0b32 2840 5110 3411 0b32 2a40 5111 3411  .2(@Q.4..2*@Q.4.
        //此处0x10无意义，仅因为测试时，似乎前16字节有错误匹配
        //需要循环查找
        pEipBegin = abWebShellCode + 0x10;
        nChildRet = 0;
        do {
            if (!(pEipBegin = MyMemSearch(pEipBegin + 1, abWebShellCode + nWebShellCodeSize, abFlag,
                                          sizeof(abFlag) - 1)))
                break;
            if (!(pEipBeginNext = MyMemSearch(pEipBegin + 1, abWebShellCode + nWebShellCodeSize,
                                              abFlag, sizeof(abFlag) - 1)))
                break;
            //找到连续的0x340x11才行，最大间隔8字节
            // example:
            // 00000000: 3411 2a30 5134 3411 2a32 f03f 5135 3411  4.*0Q44.*2.?Q54.
            if (pEipBeginNext - pEipBegin <= 8) {
                nChildRet = 1;
                break;
            }
        } while (pEipBegin != NULL);
        if (!nChildRet)
            break;  // not found
        do {
            bOpCode = *(pEipBegin + nCodeEip);
            nCodeEip += 1;
            switch (bOpCode) {
                case DPPT_JSVMPCODE_SET_KEY:
                    if ((nCodeEip = GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType,
                                                            &nOutDataValue, &bOutDataSize)) < 0) {
                        nChildRet = -1;
                        break;
                    }
                    if ((nCodeEip = GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType,
                                                            &nClearIndex, &bOutDataSize)) < 0) {
                        nChildRet = -1;
                        break;
                    }
                    if ((nCodeEip = GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType,
                                                            &nOutDataValue, &bOutDataSize)) < 0) {
                        nChildRet = -1;
                        break;
                    }
                    //直接查找
                    sprintf(szBufNum, "%d", nOutDataValue);
                    if (!(pValue = (char *)evhttp_find_header(kvStringTableAll, szBufNum))) {
                        nChildRet = -1;
                        break;
                    }
                    sprintf(szBufNum, "%d", nClearIndex);
                    evhttp_add_header(kvStringTableClear, szBufNum, pValue);
                    (*pnStringTableSize)++;
                    //_WriteLog(LL_DEBUG, "Set clearStringTable [%s]%s", szBufNum, pValue);
                    break;
                default:
                    if (*pnStringTableSize < 50) {
                        nChildRet = -1;
                        _WriteLog(LL_FATAL, "RestoreStringsTable failed, nStringTableSize<50");
                    } else
                        nChildRet = 0;
                    break;
            }
        } while (nChildRet > 0);
        if (nChildRet < 0) {
            _WriteLog(LL_FATAL, "RestoreStringsTable failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int IncrementEip(uint8 *pEipBegin, int nCodeEip, int nCount)
{
    uint8 bOutDataType = 0, bOutDataSize = 0;
    int nRet = ERR_GENERIC, nOutDataValue = 0, i = 0, nChildRet = 0;
    for (i = 0; i < nCount; i++) {
        if ((nCodeEip = GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType, &nOutDataValue,
                                                &bOutDataSize)) < 0)
            break;
    }
    return nCodeEip;
}

bool JsStackOpearte(JsStack jsStack, bool bGetOrSet, uint8 bDateType, int nIndex,
                    void *pnInOutValue, bool *pbInOutTypeStringOrNum)
{
    // bDateType == 0为栈，==208为堆
    bool bRet = false;
    char szNumKey[BS_TINY] = "", szValuePtr[BS_TINY] = "", *pValuePtr = NULL;
    struct evkeyvalq *keyq = NULL;
    struct EtaxJsvmpHeapStackItem *pItem = NULL;
    do {
        if (bDateType == 208)
            keyq = &jsStack->kvHeap;
        else {
            keyq = &jsStack->kvStack;
            //如果需要往stack0写值，传入nIndex = -fp;
            nIndex = jsStack->fp + nIndex;
        }
        sprintf(szNumKey, "%d", nIndex);
        if (bGetOrSet) {
            if (!(pValuePtr = (char *)evhttp_find_header(keyq, szNumKey)))
                break;
            if (!strcmp(pValuePtr, ""))
                break;
            sscanf(pValuePtr, "%p", &pItem);
            if (pItem->bStringOrNum)  // num
                *(struct evbuffer **)pnInOutValue = pItem->string;
            else  // string
                *(double *)pnInOutValue = pItem->num;
            *pbInOutTypeStringOrNum = pItem->bStringOrNum;
        } else {  // set
            if ((pValuePtr = (char *)evhttp_find_header(keyq, szNumKey))) {
                HeapStackMethod(jsStack, 2, pValuePtr);
                evhttp_remove_header(keyq, szNumKey);
                pItem = NULL;
            }
            if (!(pItem = (struct EtaxJsvmpHeapStackItem *)calloc(
                      1, sizeof(struct EtaxJsvmpHeapStackItem))))
                break;
            else {
                if (!(pItem->string = evbuffer_new()))
                    break;
            }
            // update
            pItem->bStringOrNum = *pbInOutTypeStringOrNum;
            if (*pbInOutTypeStringOrNum) {
                evbuffer_add_printf(pItem->string, "%s", (char *)pnInOutValue);
            } else
                pItem->num = *(double *)pnInOutValue;
            sprintf(szValuePtr, "%p", pItem);
            evhttp_add_header(keyq, szNumKey, (char *)szValuePtr);
        }
        bRet = true;
    } while (false);
    return bRet;
}

// pFinalValue返回类型多样
int GetCommandTypeSizeValue(JsStack jsStack, int nCodeEip, uint8 *pOutDataType, int *pOutDataValue,
                            uint8 *pbDataSize, void *pFinalValue, bool *bStringOrNum)
{
    char szBuf[BS_TINY], *p = NULL;
    if ((nCodeEip = GetCommandTypeSizeIndex(jsStack->abWebshellCode, nCodeEip, pOutDataType,
                                            pOutDataValue, pbDataSize)) < 0)
        return nCodeEip;
    switch (*pOutDataType) {
        case 0x30:  // 48 直接读取的值就是
            *(double *)pFinalValue = *pOutDataValue;
            break;
        case 0x50: {  // 80 原始字符串表索引取值
            bool bStringOrNumInner = true;
            sprintf(szBuf, "%d", *pOutDataValue);
            if (!(p = (char *)evhttp_find_header(jsStack->kvStringTableAll, szBuf))) {
                nCodeEip = ERR_GENERIC;
                break;
            }
            //用stack1存储值并返回
            //先存储
            JsStackOpearte(jsStack, false, 0, -jsStack->fp + 1, p, &bStringOrNumInner);
            //再读取返回
            if (!JsStackOpearte(jsStack, true, *pOutDataType, -jsStack->fp + 1, pFinalValue,
                                bStringOrNum)) {
                nCodeEip = ERR_GENERIC;
                break;
            }
        } break;
        case 0x70:  // 112 返回stack0
            if (!JsStackOpearte(jsStack, true, *pOutDataType, -jsStack->fp, pFinalValue,
                                bStringOrNum)) {
                nCodeEip = ERR_GENERIC;
                break;
            }
            break;
        default:  // 0 or 208
            if (!JsStackOpearte(jsStack, true, *pOutDataType, *pOutDataValue, pFinalValue,
                                bStringOrNum)) {
                nCodeEip = ERR_GENERIC;
                break;
            }
            break;
    }
    return nCodeEip;
}

// DPPT_JSVMPCODE_SET_CTX:
//经测试后第一个标识为DPPT_JSVMPCODE_SET_CTX前半段非jsvmp数据生成完毕,target_data
//为关键hash"ec61afa043c84aefb370c9d3ad9592be2084998630/sqlz/cssSecurity/v1/getPublicKey?t=1680246652372&los28199=a6582feee7251c95ad247bdcbcb720991680246684113"
// DPPT_JSVMPCODE_SET_KEY:
// 不断从this指针循环左移精简后的stringTable抽取字符串，生成key列表，后半段数据为SET_KEY基准测试点从此开始
int WebshellcodeFinalCalcTailString(uint8 *abWebShellCode, int nCodeEip, int nMidNumer,
                                    int nStingTablesRolHeadIndex,
                                    struct evkeyvalq *kvStringTableClear, int nStringTableSize,
                                    struct evkeyvalq *kvStringTableAll,
                                    struct evbuffer *evSha1StringOut,
                                    char *szHeadUrlTimestampString)
{
    double dValue1 = 0, dValue2 = 0;
    bool bFinish = false, bStringOrNum1, bStringOrNum2;
    int nChildRet = 0, nOutDataValue = 0, nNum1 = 0, nNum2 = 0;
    uint8 bOpCode, bExtraSha1AlgNeedFlag = 0, bOutDataSize = 0, bOutDataSize1 = 0,
                   bOutDataSize2 = 0, bOutDataType1 = 0, bOutDataType2 = 0;
    char *p = NULL, szBuf[BS_NORMAL] = "", szBuf2[BS_NORMAL] = "";
    struct evbuffer *sBufValue1 = NULL, *sBufValue2 = NULL;
    struct EtaxJsvmpStack stJsStack = {};
    void *pnInOutValue = NULL;

    JsStack jsStack = &stJsStack;
    HeapStackMethod(&stJsStack, 0, "");  //初始化
    stJsStack.fp = 17;                   //仅为模拟值，16-20都可以
    stJsStack.abWebshellCode = abWebShellCode;
    stJsStack.kvStringTableAll = kvStringTableAll;
    stJsStack.kvStringTableClear = kvStringTableClear;
    //关键操作，填充fp-4,循环shellcode中nIndex有可能为-4,此时就会调用上级堆栈,此处-4为固定值
    bStringOrNum1 = true;
    if (!JsStackOpearte(jsStack, false, 0, -4, szHeadUrlTimestampString, &bStringOrNum1)) {
        HeapStackMethod(&stJsStack, 1, "");
        return ERR_GENERIC;
    }
    do {
        //需要每次都初始化为num
        bStringOrNum1 = false;
        bStringOrNum2 = false;
        bOpCode = *(abWebShellCode + nCodeEip);
        nCodeEip += 1;
        switch (bOpCode) {
            case DPPT_JSVMPCODE_VAR:
            case DPPT_JSVMPCODE_CLS:
                nCodeEip = IncrementEip(abWebShellCode, nCodeEip, 1);
                break;
            case DPPT_JSVMPCODE_MOV:
                //第一个值不能用GetCommandTypeSizeValue，因为我们没有JSVMPCODE_VAR,可能没申请
                if ((nCodeEip = GetCommandTypeSizeIndex(abWebShellCode, nCodeEip, &bOutDataType1,
                                                        &nNum1, &bOutDataSize1)) < 0)
                    break;
                //先peek2
                nChildRet = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                    &bOutDataSize2, &sBufValue2, &bStringOrNum2);
                if (nChildRet < 0) {
                    //刚开始有些函数地址作为num传输，我们没有实现
                    if ((nCodeEip =
                             GetCommandTypeSizeIndex(jsStack->abWebshellCode, nCodeEip,
                                                     &bOutDataType2, &nNum2, &bOutDataSize2)) < 0) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    dValue2 = nNum2;  //因为函数地址不存在，随便赋值个数即可
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue2,
                                        &bStringOrNum2)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                } else {
                    if (!bStringOrNum2) {  // num
                        if ((nCodeEip = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2,
                                                                &nNum2, &bOutDataSize2, &dValue2,
                                                                &bStringOrNum2)) < 0)
                            break;
                        if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue2,
                                            &bStringOrNum2)) {
                            nCodeEip = ERR_GENERIC;
                            break;
                        }
                    } else {  // string
                        if ((nCodeEip = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2,
                                                                &nNum2, &bOutDataSize2, &sBufValue2,
                                                                &bStringOrNum2)) < 0)
                            break;
                        //此处直接使用evbuffer_pullup(sBufValue2,-1)，有可能字符串尾部粘连,必须取出重排
                        char *pNewString = calloc(1, evbuffer_get_length(sBufValue2) + 1);
                        if (!pNewString) {
                            nCodeEip = ERR_GENERIC;
                            break;
                        }
                        strncpy(pNewString, (char *)evbuffer_pullup(sBufValue2, -1),
                                evbuffer_get_length(sBufValue2));
                        nChildRet = JsStackOpearte(jsStack, false, bOutDataType1, nNum1, pNewString,
                                                   &bStringOrNum2);
                        free(pNewString);
                        if (!nChildRet) {
                            nCodeEip = ERR_GENERIC;
                            break;
                        }
                    }
                }
                break;
            case DPPT_JSVMPCODE_ADD: {
                //先peek
                nChildRet = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                    &bOutDataSize1, &sBufValue1, &bStringOrNum1);
                if (nChildRet < 0) {
                    nCodeEip = ERR_GENERIC;
                    break;
                }
                if (!bStringOrNum1) {  // num
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum1)) < 0)
                        break;
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                     &bOutDataSize2, &dValue2, &bStringOrNum2)) < 0)
                        break;
                    dValue1 += dValue2;
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue1,
                                        &bStringOrNum1)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                } else {  // string
                    if ((nCodeEip = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1,
                                                            &nNum1, &bOutDataSize1, &sBufValue1,
                                                            &bStringOrNum1)) < 0)
                        break;
                    if ((nCodeEip = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2,
                                                            &nNum2, &bOutDataSize2, &sBufValue2,
                                                            &bStringOrNum2)) < 0)
                        break;
                    evbuffer_add(sBufValue1, evbuffer_pullup(sBufValue2, -1),
                                 evbuffer_get_length(sBufValue2));
                    evbuffer_drain(evSha1StringOut, -1);
                    evbuffer_add_buffer_reference(evSha1StringOut, sBufValue1);
                }
                break;
            }
            case DPPT_JSVMPCODE_PUSH: {
                //先peek
                nChildRet = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                    &bOutDataSize1, &sBufValue1, &bStringOrNum1);
                if (nChildRet < 0) {
                    nCodeEip = ERR_GENERIC;
                    break;
                }
                if (bStringOrNum1) {  // string
                    if ((nCodeEip = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1,
                                                            &nNum1, &bOutDataSize1, &sBufValue1,
                                                            &bStringOrNum1)) < 0)
                        break;
                    if (!JsStackOpearte(jsStack, true, bOutDataType1, nNum1, &sBufValue1,
                                        &bStringOrNum1)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    p = (char *)evbuffer_pullup(sBufValue1, -1);
                } else {  // num
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum1)) < 0)
                        break;
                    nOutDataValue = dValue1;
                    //开始获取子字符串
                    nOutDataValue = nOutDataValue - nMidNumer + nStingTablesRolHeadIndex;
                    if (nOutDataValue >= nStringTableSize)
                        nOutDataValue %= nStringTableSize;
                    if (nOutDataValue < 0 || nOutDataValue > 100) {  //数值明显不对
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    sprintf(szBuf, "%d", nOutDataValue);
                    if (!(p = (char *)evhttp_find_header(kvStringTableClear,
                                                         szBuf))) {  //数值明显不对
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                }
                bStringOrNum1 = true;
                if (!JsStackOpearte(jsStack, false, 0, -jsStack->fp, p, &bStringOrNum1)) {
                    nCodeEip = ERR_GENERIC;
                    break;
                }
                //先push到栈顶，call完到栈底(stack0)
                break;
            }
            case DPPT_JSVMPCODE_CALL_REG: {
                //实际税局的js中此处call作用是将PUSH的数值字符串转换为int，并在子函数内减去中数，获得明文stringtables中索引
                //会发生eip跳转，C语言无法解释复杂js语法，因此跳过
                //一次SHA1格式
                //[bOutDataType:nOutDataValue]|208:16|48:1|144:0|....208:16|48:1|144:0|(16:3|48:1|144:0)
                // 2次次SHA1格式
                //[bOutDataType:nOutDataValue]|208:16|48:1|144:0|....208:16|48:1|144:0|(16:3|48:1|144:0)|(16:3|48:1|144:0)
                uint8 bOutDataType = 0, i = 0;
                for (i = 0; i < 3; i++) {  //每3次的第一次有差异，2、3次无差异
                    if ((nCodeEip = GetCommandTypeSizeIndex(abWebShellCode, nCodeEip, &bOutDataType,
                                                            &nOutDataValue, &bOutDataSize)) < 0)
                        break;
                    if (0 == i && 16 == bOutDataType)
                        bExtraSha1AlgNeedFlag++;
                }
                break;
            }
            case DPPT_JSVMPCODE_RET: {
                if (bExtraSha1AlgNeedFlag >= 2) {
                    memset(szBuf, 0, sizeof(szBuf));
                    CalcSHA1(evbuffer_pullup(evSha1StringOut, -1),
                             evbuffer_get_length(evSha1StringOut), szBuf);
                    memset(szBuf2, 0, sizeof(szBuf2));
                    Byte2Str(szBuf2, (uint8 *)szBuf, 20);
                    evbuffer_drain(evSha1StringOut, -1);
                    evbuffer_add(evSha1StringOut, szBuf2, strlen(szBuf2));
                }
                uint8 b0 = 0x00;
                evbuffer_add(evSha1StringOut, &b0, 1);
                bFinish = true;
                break;
            }
            default:
                nCodeEip = ERR_GENERIC;
                break;
        }
    } while (!bFinish && nCodeEip > 0);
    HeapStackMethod(&stJsStack, 1, "");
    return nCodeEip;
}

int GetTailSha1ClearTypeSubtrahend(uint8 *abFunctionTablesRawData, int nFunctionTablesDataSize,
                                   uint8 *abWebShellCode)
{
    int nRet = ERR_GENERIC, nIndex = 0, nLine = 0, nCodeEip = 0, nOutDataValue = 0;
    uint16 i = 0, nV1 = 0, nV2 = 0;
    uint8 *pBuf = NULL, bOpCode, *pEipBegin, bOutDataSize = 0;
    bool bJsvmpLoopTargetCommandFinish = false;
    //找到子函数起始地址索引
    while ((nIndex = i++ * 8) < nFunctionTablesDataSize) {
        pBuf = abFunctionTablesRawData + nIndex;
        nLine = *(int *)pBuf;
        nV1 = *(uint16 *)(pBuf + 4);
        nV2 = *(uint16 *)(pBuf + 6);
        if (nV1 != 2 || nV2 != 2)
            continue;
        pBuf = abWebShellCode + nLine;
        if (pBuf[0] != 0 || pBuf[1] != 209)
            continue;
        nRet = nLine;
        break;
    }
    if (nRet <= 0)
        return nRet;
    //伪执行代码，寻找中数
    pEipBegin = abWebShellCode + nRet;
    nRet = ERR_GENERIC;
    do {
        bOpCode = *(pEipBegin + nCodeEip);
        nCodeEip += 1;  // opcode-size
        switch (bOpCode) {
            case DPPT_JSVMPCODE_VAR:
                nCodeEip = IncrementEip(pEipBegin, nCodeEip, 1);
                break;
            case DPPT_JSVMPCODE_MOV:
                nCodeEip = IncrementEip(pEipBegin, nCodeEip, 2);
                break;
            case DPPT_JSVMPCODE_SUB: {
                bJsvmpLoopTargetCommandFinish = true;
                uint8 bOutDataType = 0;
                nCodeEip = IncrementEip(pEipBegin, nCodeEip, 1);
                if (GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType, &nOutDataValue,
                                            &bOutDataSize) < 0) {
                    nCodeEip = ERR_GENERIC;
                    break;
                }
                break;
            }
            default:
                nCodeEip = 0;
                break;
        }
    } while (!bJsvmpLoopTargetCommandFinish && nCodeEip > 0);
    if (bJsvmpLoopTargetCommandFinish && nOutDataValue > 0)
        return nOutDataValue;
    else
        return ERR_GENERIC;
}

//获取明文字符串计算主过程行号,SET_CTX后，第一个op=45，call完之后eip大变化的起始值则为该函数过程
int GetTailSha1ClearTypeMainProcessLine(uint8 *abFunctionTablesRawData, int nFunctionTablesDataSize,
                                        uint8 *abWebShellCode, int nWebShellCodeSize)
{
    int nRet = ERR_GENERIC, nIndex = 0, nLine = 0, nFuncSize = 0;
    uint16 i = 0, nV1 = 0;
    uint8 *pBuf = NULL;
    //找到子函数起始地址索引
    while ((nIndex = i++ * 8) < nFunctionTablesDataSize) {
        pBuf = abFunctionTablesRawData + nIndex;
        nLine = *(int *)pBuf;
        nV1 = *(uint16 *)(pBuf + 4);
        if (nV1 != 1)  // nV2 = *(uint16 *)(pBuf + 6); nV2没有判断意义，不固定
            continue;
        pBuf = abWebShellCode + nLine;
        //也可以使用函数GetFunctionTablesFuncSize 根据webshellcode执行内容进行函数识别
        // 1 x|0 209 16 2 209 16 17 1
        //   |0 209 15 2 209 15 17 8 36 len:129
        //   |0 209 15 2 209 15 17 8 36
        //   |0 209 15 2 209 15 17 5 36
        //   |0 209 15 2 209 15 17 5 2
        if (pBuf[0] != 0 || pBuf[1] != 209 || pBuf[3] != 2 || pBuf[4] != 209)
            continue;  //如果判断出错最后pBuf[6] != 17可以去掉
        nRet = nLine;
        break;
    }
    return nRet;
}

int GetFunctionTablesFuncSize(int nFunctionNumber, uint8 *abFunctionTablesRawData,
                              int nFunctionTablesDataSize, int nTailLine)
{
    int nCurrentFunctionLine = 0, nNextFunctionLine = 0, nNextFunctionIndex = 0, nRetSize = 0;
    do {
        nNextFunctionIndex = (nFunctionNumber + 1) * 8;
        if (nNextFunctionIndex < nFunctionTablesDataSize)
            nNextFunctionLine = *(int *)(abFunctionTablesRawData + nNextFunctionIndex);
        else
            nNextFunctionLine = nTailLine;
        nCurrentFunctionLine = *(int *)(abFunctionTablesRawData + nFunctionNumber * 8);
        nRetSize = nNextFunctionLine - nCurrentFunctionLine;
    } while (false);
    return nRetSize;
}

// true,字符串开头包含数字
int IsStringtablesStartWithNum(char *szString)
{
    uint8 bOpCode = 0;
    int i = 0, nCalcNum = -1;
    for (i = 0; i < strlen(szString); i++) {
        bOpCode = szString[i];
        if (bOpCode < 0x30 || bOpCode > 0x39)  // 0-9
            break;
        else
            nCalcNum = i;
    }
    return nCalcNum;
}

// szOutString一般最多64字节
int GetStringFromArrayWithNum(JsStack jsStack, int nNum, char *szOutString, int nBufSize)
{
    int nChildRet = 0, nRet = ERR_GENERIC;
    char szBuf[BS_NORMAL] = "", *pTargetString = NULL;
    memset(szOutString, 0, nBufSize);
    do {
        nChildRet = nNum - jsStack->nMidNumer + jsStack->nStringTablesIndex;
        if (nChildRet >= jsStack->nStringTableSize)
            nChildRet = nChildRet - jsStack->nStringTableSize;
        else if (nChildRet < 0)
            break;
        sprintf(szBuf, "%d", nChildRet);
        if (!(pTargetString = (char *)evhttp_find_header(jsStack->kvStringTableClear, szBuf)))
            break;
        strcpy(szOutString, pTargetString);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//非call调用，而是跳转操作
int JmpToRolStringNumCalc(uint8 *abWebShellCode, int nCodeEip, JsStack jsStack, int nQueryNum)
{
    int nChildRet = 0, nRet = ERR_GENERIC, nNum1 = 0, nNum2 = 0;
    double dValueParam = 0, dValue1 = 0, dValue2 = 0;
    uint8 bOpCode = 0, bOutDataSize1 = 0, bOutDataSize2 = 0, bOutDataType1 = 0, bOutDataType2 = 0,
          bPushCount = 0;
    char szBuf[BS_NORMAL] = "";
    bool bCalcFinish = false, bAfterCallCtxResult = false, bStringOrNum = false;
    do {
        if ((nChildRet = GetStringFromArrayWithNum(jsStack, nQueryNum, szBuf, sizeof(szBuf))) < 0) {
            nCodeEip = ERR_GENERIC;
            break;
        }
        // _WriteLog(LL_DEBUG,
        //           "JmpToRolString NumCalc Stack.nStringTablesIndex:%d child check string:%s",
        //           jsStack->nStringTablesIndex, szBuf);
        if ((nChildRet = IsStringtablesStartWithNum(szBuf)) < 0) {
            nCodeEip = 0;  //不符合非存在问题，返回0
            break;
        }
        szBuf[nChildRet + 1] = '\0';
        dValueParam = atoi(szBuf);
        do {
            bOpCode = *(abWebShellCode + nCodeEip);
            nCodeEip += 1;  // opcode-size
            switch (bOpCode) {
                case DPPT_JSVMPCODE_CALL_REG:
                    nCodeEip = IncrementEip(abWebShellCode, nCodeEip, 3);
                    break;
                case DPPT_JSVMPCODE_CALL_CTX:
                    // bOpCode==43，字符串转int，官方转换不了则返回N/A
                    // push字符串转int后数值放在堆栈上
                    nCodeEip = IncrementEip(abWebShellCode, nCodeEip, 3);
                    bAfterCallCtxResult = true;
                    break;
                case DPPT_JSVMPCODE_MINUS:
                    // 56==minus,lastValue取负数 208 14
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum)) < 0)
                        break;
                    dValue1 = 0 - dValue1;
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue1,
                                        &bStringOrNum))
                        return ERR_GENERIC;
                    break;
                case DPPT_JSVMPCODE_DIV:
                    // 6==div 取lastValue,datetype:208,index:14
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum)) < 0)
                        break;
                    //除以第二个数字
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                     &bOutDataSize2, &dValue2, &bStringOrNum)) < 0)
                        break;
                    dValue1 /= dValue2;
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue1,
                                        &bStringOrNum))
                        return ERR_GENERIC;
                    break;
                case DPPT_JSVMPCODE_MOV:
                    //第一个值不能用GetCommandTypeSizeValue，因为我们没有JSVMPCODE_VAR,可能没申请
                    if ((nCodeEip = GetCommandTypeSizeIndex(
                             abWebShellCode, nCodeEip, &bOutDataType1, &nNum1, &bOutDataSize1)) < 0)
                        break;
                    nChildRet = GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                        &bOutDataSize2, &dValue2, &bStringOrNum);
                    if (bAfterCallCtxResult) {  //外部计算值介入，覆盖堆栈
                        dValue2 = dValueParam;
                        bAfterCallCtxResult = false;
                    }
                    if (nChildRet > 0) {
                        nCodeEip = nChildRet;
                    } else {
                        if ((nCodeEip = GetCommandTypeSizeIndex(jsStack->abWebshellCode, nCodeEip,
                                                                &bOutDataType2, &nNum2,
                                                                &bOutDataSize2)) < 0)
                            break;
                    }
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue2,
                                        &bStringOrNum)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    break;
                case DPPT_JSVMPCODE_ADD:
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum)) < 0)
                        break;
                    //除以第二个数字
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                     &bOutDataSize2, &dValue2, &bStringOrNum)) < 0)
                        break;
                    dValue1 += dValue2;
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue1,
                                        &bStringOrNum)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    break;
                case DPPT_JSVMPCODE_PUSH:
                    //每次循环两次push，我们只需要第2次的即可
                    //第一次是push call查找到的字符串，第二次是将查找到的字符串push到堆栈上，
                    if (++bPushCount % 2 == 0) {
                        nCodeEip -= 1;  //返还指令，交给外面继续循环,提取检测
                        bCalcFinish = true;
                    } else {
                        if ((nCodeEip = GetCommandTypeSizeIndex(jsStack->abWebshellCode, nCodeEip,
                                                                &bOutDataType1, &nNum1,
                                                                &bOutDataSize1)) < 0)
                            break;
                    }
                    break;
                case DPPT_JSVMPCODE_MUL:
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum)) < 0)
                        break;
                    //除以第二个数字
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                     &bOutDataSize2, &dValue2, &bStringOrNum)) < 0)
                        break;
                    dValue1 *= dValue2;
                    if (!JsStackOpearte(jsStack, false, bOutDataType1, nNum1, &dValue1,
                                        &bStringOrNum)) {
                        nCodeEip = ERR_GENERIC;
                        break;
                    }
                    break;
                case DPPT_JSVMPCODE_EQ:
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType1, &nNum1,
                                                     &bOutDataSize1, &dValue1, &bStringOrNum)) < 0)
                        break;
                    if ((nCodeEip =
                             GetCommandTypeSizeValue(jsStack, nCodeEip, &bOutDataType2, &nNum2,
                                                     &bOutDataSize2, &dValue2, &bStringOrNum)) < 0)
                        break;
                    //
                    bCalcFinish = true;
                    nCodeEip = 0;
                    if (dValue1 == jsStack->nRolFinishCheckNum)
                        jsStack->bMatchFlag = true;
                    break;
                default:
                    nCodeEip = -1;
                    break;
            }
        } while (nCodeEip > 0 && !bCalcFinish);
    } while (false);
    return nCodeEip;
}

//此处的操作的peek shellcode，不影响外部nCodeEip的数值
int RolStringPushDo(uint8 *abWebShellCode, int nCodeEip, JsStack jsStack)
{
    int nChildRet = 0, nNum1 = 0, nNum2 = 0;
    double dOutDataValue = 0, dOutDataValue2 = 0;
    uint8 bOpCode = 0, bOutDataSize = 0, bOutDataSize2 = 0, bOutDataType = 0, bOutDataType2 = 0;
    char szBuf[BS_NORMAL] = "";
    bool bStringOrNum = false;
    do {
        //--------------全函数合计2*x个push指令，以第一个push为切分点分割为多个segment---------------------
        if ((nCodeEip = GetCommandTypeSizeIndex(abWebShellCode, ++nCodeEip, &bOutDataType, &nNum1,
                                                &bOutDataSize)) < 0)
            break;
        if (0x30 == bOutDataType) {
            //获取的就是直接值，可以直接用
        } else if (0 == bOutDataType) {
            dOutDataValue2 = 0;
            JsStackOpearte(jsStack, true, 0, nNum1, &dOutDataValue2, &bStringOrNum);
            nNum1 = dOutDataValue2;
        }
        if ((nCodeEip = JmpToRolStringNumCalc(abWebShellCode, nCodeEip, jsStack, nNum1)) <= 0)
            break;
    } while (true);
    return nCodeEip;
}

int RolStringLoopCheck(uint8 *abWebShellCode, int nFunctionStartIndex, int nFunctionSize,
                       JsStack jsStack)
{
    int nCodeEip = 0;
    uint8 bOpCode = 0, *pEipBegin = NULL;
    pEipBegin = abWebShellCode + nFunctionStartIndex;
    //前几十个字节作用不大，避免干扰，跳过
    if (!(pEipBegin =
              MyMemSearch(pEipBegin + 30, pEipBegin + nFunctionSize, (uint8 *)"\x41\x82", 2)))
        return ERR_GENERIC;
    nCodeEip = pEipBegin - abWebShellCode;
    do {
        bOpCode = *(abWebShellCode + nCodeEip);
        nCodeEip += 1;  // opcode-size
        switch (bOpCode) {
            case DPPT_JSVMPCODE_TRY:
                // example1
                // :	65	130	15	3	130	62	3	36
                // :	51	192	99	64	45	209	11	50
                // example2
                // :	65	130	194	4	130	241	4	36
                // :	51	240	114	64	45	209	12	50
                // example3
                // :	65	130	115	3	130	162	3	36
                // :	51	32	110	64	45	209	12	50
                nCodeEip = IncrementEip(abWebShellCode, nCodeEip, 2);
                break;
            case DPPT_JSVMPCODE_PUSH:
                //每次循环两次push，第一次是push查找字符串，第二次是将查找到的字符串push到堆栈上，我们只需要第一次的即可
                // nCodeEip返还一个字节指令给内部调用
                nCodeEip = RolStringPushDo(abWebShellCode, nCodeEip - 1, jsStack);
                break;
            default:
                nCodeEip = -1;
                break;
        }
    } while (nCodeEip > 0 && nCodeEip < (nFunctionStartIndex + nFunctionSize));
    if (nCodeEip < 0)
        return nCodeEip;
    //>=0
    if (jsStack->bMatchFlag)
        return 1;
    else
        return 0;
}

int JsvmpGetStringTablesIndex(uint8 *abWebShellCode, int nFunctionStartIndex, int nFunctionEndIndex,
                              struct evkeyvalq *kvStringTableClear, int nMidNumer,
                              int nStringTableSize, int nRolFinishCheckNum)
{
    int nChilRet = 0, nRet = ERR_GENERIC;
    double dVar = 0.1;
    bool bStringOrNum = false;
    struct evkeyval *kv = NULL;
    struct EtaxJsvmpStack stJsStack = {};
    HeapStackMethod(&stJsStack, 0, "");
    stJsStack.nStringTableSize = nStringTableSize;
    //填充堆栈准备计算，模拟官方堆栈
    stJsStack.kvStringTableClear = kvStringTableClear;
    //--这个中位数，实际在下面的子函数调用时也可以获取，为了省事，我们就用外部已经获取到的
    stJsStack.nMidNumer = nMidNumer;
    stJsStack.nRolFinishCheckNum = nRolFinishCheckNum;
    JsStackOpearte(&stJsStack, false, 0, 0, &dVar, &bStringOrNum);  // stack 0 protect
    stJsStack.fp = 17;
    stJsStack.abWebshellCode = abWebShellCode;
    //本次仅仅起到遍历作用
    TAILQ_FOREACH(kv, kvStringTableClear, next)
    {
        nChilRet = RolStringLoopCheck(abWebShellCode, nFunctionStartIndex,
                                      nFunctionEndIndex - nFunctionStartIndex, &stJsStack);
        if (nChilRet < 0)
            break;
        else if (nChilRet > 0) {
            nRet = stJsStack.nStringTablesIndex;
            break;
        }
        //==0则继续左移查询，stringtables左移动检测主循环
        stJsStack.nStringTablesIndex++;
    }
    HeapStackMethod(&stJsStack, 1, "");
    return nRet;
}

//循环左移字符串表
int RolClearTypeStringTable(uint8 *abFunctionTablesRawData, int nFunctionTablesDataSize,
                            uint8 *abWebShellCode, int nWebShellCodeSize,
                            struct evkeyvalq *kvStringTableClear, int nMidNumer,
                            int nStringTableSize)
{
    int nRet = ERR_GENERIC, nLine = 0, nIndex = 0, nCodeEip = 0, nRolFinishCheckNum = 0,
        nFuncSize = 0;
    uint16 i = 0, nV1 = 0;
    uint8 *pBuf = NULL, bOpCode, *pEipBegin;
    bool bJsvmpLoopTargetCommandFinish = false;
    // 1)获取循环结束对比数(起初设计时用到，后续发现用不上，代码先保留)
    //#该模块代码位于程序webshellcode刚开始好像是前几个push时出现
    pEipBegin = abWebShellCode;
    do {
        bOpCode = *(pEipBegin + nCodeEip);
        nCodeEip += 1;  // opcode-size
        switch (bOpCode) {
            case DPPT_JSVMPCODE_VAR:
            case DPPT_JSVMPCODE_CLS:
                nCodeEip = IncrementEip(pEipBegin, nCodeEip, 1);
                break;
            case DPPT_JSVMPCODE_FUNC:
            case DPPT_JSVMPCODE_MOV:
                nCodeEip = IncrementEip(pEipBegin, nCodeEip, 2);
                break;
            case DPPT_JSVMPCODE_PUSH: {
                uint8 bOutDataType, bOutDataSize;
                nCodeEip = GetCommandTypeSizeIndex(pEipBegin, nCodeEip, &bOutDataType,
                                                   &nRolFinishCheckNum, &bOutDataSize);
                break;
            }
            case DPPT_JSVMPCODE_CALL_REG:
                bJsvmpLoopTargetCommandFinish = true;
                break;
            default:
                nCodeEip = 0;
                break;
        }
    } while (!bJsvmpLoopTargetCommandFinish && nCodeEip > 0);
    if (!bJsvmpLoopTargetCommandFinish || nRolFinishCheckNum > 1000000 || nRolFinishCheckNum < 0)
        return ERR_GENERIC;  // nRolFinishCheckNum 一般6位数值，example:303077
    // 2)找到子函数webshell所在代码块
    //#2-4步骤位于逻辑模块try end中实现，每try end一次，循环左移一次;swtict-Num==65(Try)所在函数块
    while ((nIndex = i * 8) < nFunctionTablesDataSize) {
        pBuf = abFunctionTablesRawData + nIndex;
        nFuncSize = GetFunctionTablesFuncSize(i++, abFunctionTablesRawData, nFunctionTablesDataSize,
                                              nWebShellCodeSize);
        nLine = *(int *)pBuf;
        nV1 = *(uint16 *)(pBuf + 4);
        if (nV1 != 2)
            continue;
        pBuf = abWebShellCode + nLine;
        if (pBuf[0] != 0 || pBuf[1] != 209 || pBuf[3] != 0 || pBuf[4] != 209)
            continue;
        //实测函数542-667字节
        // example1
        // :	0	209	11	0	209	12	0	209
        // :	13	2	209	11	17	5	45	4
        // :	252	255	255	255	48	144	2	209
        // :	12	112	75	50	240	63	50	1
        if (nFuncSize < 500 || nFuncSize > 720)
            continue;
        nRet = nLine;
        break;
    }
    if (nRet <= 0)
        return nRet;
    // 3)找到子函数起始地址索引，nRolFinishCheckNum为webshellcode刚开始执行的时候传入循环结束比对数
    if ((nIndex =
             JsvmpGetStringTablesIndex(abWebShellCode, nRet, nRet + nFuncSize, kvStringTableClear,
                                       nMidNumer, nStringTableSize, nRolFinishCheckNum)) < 0)
        return ERR_GENERIC;
    // 4)将索引值所在数组中的字符串左循环移动到首位,目前返回索引
    return nIndex;
}

// szRepCode419StringB64为服务器的base64响应，szHeadString为前面已经拼接好的前置固定字符串,
//返回值为前置和后生成的全部sring
int GetSha1ClearString(char *szRepCode419StringB64, char *szHeadUrlTimestampString,
                       struct evbuffer *evSha1StringOut)
{
    // pBuf载荷，前16字节为索引，第一段为webshellcode，第二段函数表，第三段为字符串表
    // index(16bytes)|code|function-tables|string-tables(乱序)
    //逻辑断点从大约16800|16308行左右,搜索'= 0x1'，'let _0xbd2d35 = 0x1;'
    int nRet = ERR_GENERIC, nMaxBufSize = 16 * BS_BLOCK, nDeBufSize = 0, nChildRet = 0, nTmpVar = 0,
        nFunctionTablesDataSize = 0, nMidNumer = 0, nTailSha1ClearTypeMainLine = 0,
        nStingTablesRolHeadIndex = 0, nStringTableSize = 0;
    uint8 *pBuf = NULL, abFunctionTablesRawData[BS_BIG] = "", *abWebShellCode = NULL;
    struct evkeyvalq kvStringTableAll, kvStringTableClear;  //连同代码段的字符串都混在里面
    TAILQ_INIT(&kvStringTableAll);
    TAILQ_INIT(&kvStringTableClear);
    do {
        if (!(pBuf = calloc(1, nMaxBufSize)))
            break;
        nChildRet = strlen(szRepCode419StringB64);
        if (nChildRet > nMaxBufSize)
            break;
        //==base64decode
        if ((nDeBufSize = Base64_Decode(szRepCode419StringB64, nChildRet, (char *)pBuf)) < 0)
            break;
        abWebShellCode = pBuf + 0x10;
        //==获取函数表,一般100多个字节
        nFunctionTablesDataSize = *(int *)(pBuf + 8) - *(int *)(pBuf + 4);
        if (nFunctionTablesDataSize > BS_BIG)
            break;
        memcpy(abFunctionTablesRawData, pBuf + *(int *)(pBuf + 4), nFunctionTablesDataSize);
        //==1获取乱序字符串表
        nTmpVar = *(int *)(pBuf + 8);  //字符串表索引数值
        if (!FormatGetStringsTable(pBuf + nTmpVar, nDeBufSize - nTmpVar, &kvStringTableAll)) {
            _WriteLog(LL_FATAL, "FormatGetStringsTable failed");
            break;
        }
        //==2抽取代码段，伪执行，还原乱序字符串表
        nTmpVar = *(int *)(pBuf + 4);  //函数表索引数值
        if (RestoreStringsTable(abWebShellCode, nTmpVar - 0x10, &kvStringTableAll,
                                &kvStringTableClear, &nStringTableSize)) {
            _WriteLog(LL_FATAL, "RestoreStringsTable failed");
            break;
        }
        //==获取后128字节sha1明文抽取索引被减值
        if ((nMidNumer = GetTailSha1ClearTypeSubtrahend(
                 abFunctionTablesRawData, nFunctionTablesDataSize, abWebShellCode)) < 0) {
            _WriteLog(LL_FATAL, "GetTailSha1ClearTypeSubtrahend failed");
            break;
        }
        //==3循环左移字符串表
        if ((nStingTablesRolHeadIndex = RolClearTypeStringTable(
                 abFunctionTablesRawData, nFunctionTablesDataSize, abWebShellCode, nTmpVar - 0x10,
                 &kvStringTableClear, nMidNumer, nStringTableSize)) < 0) {
            _WriteLog(LL_FATAL, "RolClearTypeStringTable failed");
            break;
        }
        //==获取明文生成主流程行号
        if ((nTailSha1ClearTypeMainLine = GetTailSha1ClearTypeMainProcessLine(
                 abFunctionTablesRawData, nFunctionTablesDataSize, abWebShellCode,
                 nTmpVar - 0x10)) < 0) {
            _WriteLog(LL_FATAL, "GetTailSha1ClearTypeMainProcessLine failed");
            break;
        }
        //==栈解析后128位hash字符串
        if (WebshellcodeFinalCalcTailString(
                (uint8 *)pBuf + 0x10, nTailSha1ClearTypeMainLine, nMidNumer,
                nStingTablesRolHeadIndex, &kvStringTableClear, nStringTableSize, &kvStringTableAll,
                evSha1StringOut, szHeadUrlTimestampString) < 0) {
            _WriteLog(LL_FATAL, "RolStringLoopCheck failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (pBuf)
        free(pBuf);
    evhttp_clear_headers(&kvStringTableAll);
    evhttp_clear_headers(&kvStringTableClear);
    return nRet;
}

void GetMd5Sm3HashString(char *szInstring, char *szOutBuf)
{
    char szBuf1[BS_BLOCK] = "", szBuf2[BS_NORMAL] = "";
    memset(szBuf2, 0, sizeof(szBuf2));
    CalcMD5(szInstring, strlen(szInstring), szBuf2);
    memset(szBuf1, 0, sizeof(szBuf1));
    Byte2Str(szBuf1, (uint8 *)szBuf2, 16);
    memset(szBuf2, 0, sizeof(szBuf2));
    CalcSM3((uint8 *)szBuf1, strlen(szBuf1), (uint8 *)szBuf2);
    memset(szBuf1, 0, sizeof(szBuf1));
    Byte2Str(szOutBuf, (uint8 *)szBuf2, 32);
}

// llTimeStampMs实际可以不需要该参数，为了调试方便，先保留;szPostDataString可为空字符串
int GetLzkqow23819Header(char *sz28199UrlPath, char *szCookieDzfpSsotoken,
                         char *szRepCode419StringB64, char *szPostDataString, uint64 llTimeStampMs,
                         uint64 llFingerPrint, char *szOutString)
{
    // 最后4个32字节，每个分为3*10+2,128位为从随机取值（由jsmp引擎根据服务器给的shellcode计算获得）
    // [dzfp-ssotoken][fingerprint][szOrigUrl][timestamp]{[rand32bytes-1][rand32bytes-2][rand32bytes-3][rand32bytes-4]}
    //
    // 1)明文cb670ec3739c47889d9b8293a74527d21119921094/szzhzz/cssSecurity/v1/getPublicKey?t=1679935882124&los28199=4e317ea7bfd5358827a3ca8d3c28292916799359004581dce3942871342cfa58b8a9ce7a3ec179e5cafef6f01404196eab339e10ae348ed6588f7437d44148f0b898d034cf8fb9fc534d532014e2c981e4da00ab73138
    // 2)sha1:e9660deceb8d572054395b12d0d34885639e17fb
    // 3)md5:4ce573f45a60b92ec81d099826989286
    // 4)sm3:1ea9b055aaa69febce4fbe5459ac6ede63446960a843b5e266cfb2f692a6ce7c[lzkqow23819-key-random]
    int nRet = ERR_GENERIC, nChildRet = ERR_GENERIC;
    struct evbuffer *evSha1String = NULL;
    char szBuf1[BS_BLOCK] = "", szBuf2[BS_NORMAL] = "", szinalSm3Hash[BS_NORMAL] = "", *p = NULL;
    do {
        if ((nChildRet = snprintf(szBuf1, sizeof(szBuf1) - 1, "%s%llu%s%llu", szCookieDzfpSsotoken,
                                  llFingerPrint, sz28199UrlPath, llTimeStampMs)) < 64)
            break;
        if (!(evSha1String = evbuffer_new()))
            break;
        if (szRepCode419StringB64 && strlen(szRepCode419StringB64)) {
            if (GetSha1ClearString(szRepCode419StringB64, szBuf1, evSha1String))
                break;
            p = (char *)evbuffer_pullup(evSha1String, -1);
        } else  //首次，直接给个错误的sha1明文，发送给服务器以获取新的响应，再次执行得到正确结果
            p = szCookieDzfpSsotoken;
        //都是小写字符串进行hash
        memset(szBuf2, 0, sizeof(szBuf2));
        if (CalcSHA1(p, strlen(p), szBuf2))
            break;
        memset(szBuf1, 0, sizeof(szBuf1));
        Byte2Str(szBuf1, (uint8 *)szBuf2, 20);
        GetMd5Sm3HashString(szBuf1, szinalSm3Hash);
        if (!strlen(szPostDataString)) {
            nRet = RET_SUCCESS;
            break;
        }
        // 如果有post数据的还要继续往下计算，lzkqow23819 example(正常最大长度90字节左右):
        // 1119921094.f0b21ad396f48a3e933d8cd8f21d6517e858c4a9df1e9c46db2e1eeab4f8aad4.1679935525373
        memset(szBuf1, 0, sizeof(szBuf1));
        GetMd5Sm3HashString(szPostDataString, szBuf1);
        strcat(szBuf1, szinalSm3Hash);
        memset(szinalSm3Hash, 0, sizeof(szinalSm3Hash));
        GetMd5Sm3HashString(szBuf1, szinalSm3Hash);
        nRet = RET_SUCCESS;
    } while (false);
    if (evSha1String)
        evbuffer_free(evSha1String);
    //没有post数据的，直接URL计算出来则结束
    if (!nRet)
        sprintf(szOutString, "%llu.%s.%llu", llFingerPrint, szinalSm3Hash, llTimeStampMs);
    else
        _WriteLog(LL_WARN, "GetLzkqow23819Header failed");
    return nRet;
}