#ifndef CNTAX_REPORT_CLEAR_H
#define CNTAX_REPORT_CLEAR_H
#include "cntax_common.h"

//老的组件抄报清卡流程
//wang 04 03 去除老组件 有关调用老组件的接口注释取消
#if CntaxReport
int CntaxReportSummarytMain(HDEV hDev, uint8 bFplx_Common);
int CntaxClearCardMain(HDEV hDev, uint8 bFplx_Common);
#endif

//新的开票软件，抄报清卡流程(测试接口,稳定后替换老接口)
int CntaxReportSummarytMain(HDEV hDev,bool judge_flag);
int CntaxClearCardMain(HDEV hDev);

#endif
