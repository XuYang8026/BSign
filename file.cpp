#include "file.h"

file::file()
{

}

bool isFileExist(QString filePath){
    QFileInfo fileInfo(filePath);
    return fileInfo.exists();
}
