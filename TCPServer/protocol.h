#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned int uint;  //无符号整数

//定义宏
#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed: Username already exists!"

#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed: The username and password do not match, or you have already logged in"

#define SEARCH_USER_NO "no such people"
#define SEARCH_USER_ONLINE "This person is online~"
#define SEARCH_USER_OFFLINE "This person is offline!"

#define UNKNOW_ERROR "unknow errror" //未知错误
#define EXISTED_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "user offline"
#define ADD_FRIEND_NOEXIST "user no exist"
#define DELETE_FRIEND_OK "delete friend ok"

#define DIR_EXIST "directory already exists"
#define DIR_CREATE_OK "Successfully created directory"
#define DIR_CREATE_FAILED "Failed to create a new directory"

#define DELETE_FILE_OK "Successfully deleted file"
#define DELETE_FILE_FAILED "Failed to delete file, it may not exist"

#define RENAME_FILE_OK "Renaming file successful"
#define RENAME_FILE_FAILED "Renaming file failed"

#define FILE_EXIST "file already exists"
#define FILE_CREATE_OK "Successfully created file"
#define FILE_CREATE_FAILED "Failed to create a new file"

#define FILE_UPLOAD_OK "File uploaded successfully"
#define FILE_UPLOAD_FAILED "File upload failed"

#define FILE_MOVE_OK "File move successfully"
#define FILE_MOVE_FAILED "File move failed"

//枚举消息类型
enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_REGIST_REQUEST,//注册请求
    ENUM_MSG_REGIST_RESPOND,//注册回复

    ENUM_MSG_LOGIN_REQUEST,//登录请求
    ENUM_MSG_LOGIN_RESPOND,//登录回复

    ENUM_MSG_ALL_ONLINE_REQUEST,//所有在线用户请求
    ENUM_MSG_ALL_ONLINE_RESPOND,

    ENUM_MSG_SEARCH_USERS_REQUEST,//搜索用户请求
    ENUM_MSG_SEARCH_USERS_RESPOND,

    ENUM_MSG_ADD_FRIEND_REQUEST,//添加好友请求
    ENUM_MSG_ADD_FRIEND_RESPOND,

    ENUM_MSG_ADD_FRIEND_AGREE,//同意添加好友
    ENUM_MSG_ADD_FRIEND_REFUSE,//拒绝添加好友

    ENUM_MSG_FLUSH_FRIEND_REQUEST,//刷新好友请求
    ENUM_MSG_FLUSH_FRIEND_RESPOND,

    ENUM_MSG_DELETE_FRIEND_REQUEST,//删除好友请求
    ENUM_MSG_DELETE_FRIEND_RESPOND,

    ENUM_MSG_PRIVATE_CHAT_REQUEST,//私聊请求
    ENUM_MSG_PRIVATE_CHAT_RESPOND,

    ENUM_MSG_GROUP_CHAT_REQUEST,//群聊请求
    ENUM_MSG_GROUP_CHAT_RESPOND,

    ENUM_MSG_CREATE_DIR_REQUEST,//创建目录请求
    ENUM_MSG_CREATE_DIR_RESPOND,

    ENUM_MSG_FLUSH_DIR_REQUEST,//刷新目录请求
    ENUM_MSG_FLUSH_DIR_RESPOND,

    ENUM_MSG_DELETE_FILE_REQUEST,//删除文件或目录请求
    ENUM_MSG_DELETE_FILE_RESPOND,

    ENUM_MSG_RENAME_FILE_REQUEST,//重命名文件或目录请求
    ENUM_MSG_RENAME_FILE_RESPAND,

    ENUM_MSG_CREATE_FILE_REQUEST,//新建文件请求
    ENUM_MSG_CREATE_FILE_RESPAND,

    ENUM_MSG_UPLOAD_FILE_REQUEST,//上传文件请求
    ENUM_MSG_UPLOAD_FILE_RESPAND,

    ENUM_MSG_DOWNLOAD_FILE_REQUEST,//下载文件请求
    ENUM_MSG_DOWNLOAD_FILE_RESPAND,

    ENUM_MSG_SHARE_FILE_REQUEST,//共享文件请求
    ENUM_MSG_SHARE_FILE_RESPAND,

    ENUM_MSG_SHARE_FILE_NOTICE,//共享文件通知
    ENUM_MSG_SHARE_FILE_NOTICE_RESPOND,//共享文件通知的回复

    ENUM_MSG_MOVE_FILE_REQUEST,//移动（剪切和粘贴）文件请求
    ENUM_MSG_MOVE_FILE_RESPAND,

    ENUM_MSG_LOGOFF_REQUEST,//注销请求
    ENUM_MSG_LOGOFF_RESPAND,//注销回复

    ENUM_MSG_TYPE_MAX=0X00ffffff
};

//协议结构体
struct PDU  //协议数据单元
{
    uint uiPDULen;  //总的协议数据单元大小
    uint uiMsgType; //消息类型
    char caData[64]; //文件名
    uint uiMsgLen; //实际消息长度
    int caMsg[]; //实际消息
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
