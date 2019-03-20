#include "ithread.h"
#include "signutil.h"

IThread::IThread(){}

void IThread::run(){
    emit execPrint("正在读取IPA信息...");
    SignUtil signUtil;
    signUtil.readIpaInfo(this->filePath);
    emit execPrint("IPA信息读取完成");
    emit send(signUtil.ipaInfo);
}

