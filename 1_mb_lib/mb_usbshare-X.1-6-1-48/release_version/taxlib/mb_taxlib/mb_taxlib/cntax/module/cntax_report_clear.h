#ifndef CNTAX_REPORT_CLEAR_H
#define CNTAX_REPORT_CLEAR_H
#include "cntax_common.h"

//�ϵ���������忨����
//wang 04 03 ȥ������� �йص���������Ľӿ�ע��ȡ��
#if CntaxReport
int CntaxReportSummarytMain(HDEV hDev, uint8 bFplx_Common);
int CntaxClearCardMain(HDEV hDev, uint8 bFplx_Common);
#endif

//�µĿ�Ʊ����������忨����(���Խӿ�,�ȶ����滻�Ͻӿ�)
int CntaxReportSummarytMain(HDEV hDev,bool judge_flag);
int CntaxClearCardMain(HDEV hDev);

#endif
