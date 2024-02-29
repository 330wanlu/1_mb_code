#ifndef ETAX_COMMON_H
#define ETAX_COMMON_H

//先不建etax_define文件，单独放在此处

enum enEtaxOperate {
    // 1-999内部API，禁止调用
    ETAX_HELLO = 0,
    ETAX_QUERYMM,
    ETAX_LOGIN,
    ETAX_LOGOUT,
    ETAX_SPLIT_LINE = 999,  //分割线
    //如下为业务逻辑调用编号
    //--旧版本确认平台
    ETAX_SWSXTZS = 1000,  //系统辅助功能-税务事项通知书查询
    ETAX_FPXZ,            //发票管理-发票下载
    ETAX_HZFP,            //登陆提醒-红字发票查询
    //--新版本电子税务局
    ETAX_NSRXX,  //获取纳税人信息
    ETAX_FPCX,   //发票全量查询
};
typedef enum enEtaxOperate ETaxFlag;

#include "../common/tax_common.h"
#include "etax_auth_server.h"
#include "etax_bridge.h"
#include "etax_confirm.h"

#endif