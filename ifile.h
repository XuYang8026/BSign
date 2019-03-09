#ifndef IFILE_H
#define IFILE_H

#include "QDir"
#include "QFileInfoList"
#include "QFileInfoList"

QFileInfoList GetFileList(QString path);

bool CopyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist);

class IFile
{
public:
    IFile();
};

#endif // IFILE_H
