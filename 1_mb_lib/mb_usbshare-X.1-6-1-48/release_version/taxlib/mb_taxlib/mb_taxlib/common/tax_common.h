/*****************************************************************************
File name:   common.h
Description: 全局头文件引用
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20191201     初始化
*****************************************************************************/
#ifndef TAX_COMMON_H
#define TAX_COMMON_H
#include "define.h"
#include "common.h"
//---------------------------本地第三方依赖---------------------------------
#include "../3rdlib/cjson/cJSON.h"
#include "../3rdlib/mxml/mxml.h"
//--------------------------本地模块头文件-----------------------------------
//##调用先后顺序不能调整，否则需要调整头文件引用
// clang-format off
#include "tax_function.h"  //涉及aisino和nisec两个工程的连接，因此必须放在最后引用
#include "tax_debug.h"
#include "tax_unit_test.h"
//##不分先后 begin
#include "importinv.h"
#include "fpxx_json.h"
//##不分先后 finish
//通用实现调用子模块是不太合适的，因为不是C++工程，无法用类来进行继承，但是为了减少代码冗余先这么实现吧
#include "../authority/ta_iface.h"
#include "../aisino/module/aisino_common.h"
#include "../nisec/module/nisec_common.h"
#include "../cntax/module/cntax_common.h"
#include "../etax/etax_common.h"

// clang-format on


#endif  //_COMMON_HEADER_H
