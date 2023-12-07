# 基于QT框架的网盘系统

## 项目介绍

- 基于QT的网盘系统，实现的功能有用户注册、登录、注销、添加好友、删除好友、私聊、群聊、搜索好友、刷新目录、进入目录、返回、删除文件、上传文件、下载文件等，
- 分为客户端和服务器端，两个大的功能模块是好友操作和文件操作，用户的个人信息和好友信息是存储在SQLite数据库中，文件操作的文件是存储在服务器端，
- 设计通信协议，用于接收客户端不同的请求，比如添加好友、删除好友、上传文件、下载文件等，通信协议中还使用了弹性结构体，不同类型的消息申请不同的空间大小，节省内存，提高效率，
- 服务器端使用面向对象编程自定义mytcpsocket类用于和客户端的网络通信，以及与数据库操作、文件操作，
- 客户端在用户注册时对用户的注册密码进行哈希加密，提高用户隐私安全性，客户端还提供了较友好美观的用户界面，如选项卡式界面（用户可以通过点击选项卡（或标签页）来切换不同的窗口或视图），右键菜单（用户通过右键点击弹出菜单供用户选择操作）等。



## 项目的技术要点

1. QT编程
2. TCP Socket网络编程
3. SQLite3数据库
4. 面向对象编程

## 项目需求

1. 用户注册、登录、注销。
2. 好友操作：刷新好友列表，添加好友，删除好友，搜索用户，显示在线用户，私聊，群聊等功能。
3. 文件操作：刷新目录，上传文件，下载文件，删除文件，进入目录，返回等功能。



## 项目过程

1. 安装 SQLite

     

2. 设计数据表 

用户信息表`userInfo`
|字段|类型|约束条件|其他|
|--|--|--|--|
|id|integer|primary key|autoincrement|
|name|varchar(32)|not null||
|pwd|varchar(32)|not null||

用户好友表`friend`
|字段|类型|约束条件|其他|
|--|--|--|--|
|id|integer|主键|外键|
|friendId|integer|主键|外键|

3. 安装Qt  

问题："No suitable kits found" = 没有找到合适的kits套件，在安装Qt Creator时没有安装MinGW，所以只需要进行安装即可。  

解决方法：选择安装目录下的“MaintenanceTool.exe”，双击计入组件安装界面，根据自己安装的版本选择MinGW组件，点击下一步。重启后就会出现 kits。  

  

4. 配置文件的加载

- 代码目录下创建配置文件：IP，端口

- 作为资源文件添加入项目，添加前缀，添加文件

- 读取文件，处理数据

   

5. tcp客户端连接服务器  

加载配置文件->产生socket->连接服务器（成功连接服务器会发出connected()信号，以此判断是否成功连接服务器）  

->接收服务器数据  

->发送数据给服务器

 

6. 服务器的实现

- 加载配置文件

- 连接数据库

- 接收客户端的连接   

加载配置文件及打开数据库->QTcpServer监听（自定义类并继承QTcpServer）->循环接收客户端连接（服务器永不停机->获取新QTcpSocket->与客户端数据交互及数据库操作  



7. 通信协议设计

- 弹性结构体：弹性数组  

通信协议的设计  

uiPDULen(protocol data unity 协议数据单元)：发送数据总的大小  

uiMsgTpye：消息类型，如注册，登录，加好友  

uiMsgLen：实际消息大小  

```cpp
//协议结构体
struct PDU  //协议数据单元
{
    uint uiPDULen;  //总的协议数据单元大小
    uint uiMsgType; //消息类型
    char caData[64]; //文件名
    uint uiMsgLen; //实际消息长度
    int caMsg[]; //实际消息
};

```

收发数据  

服务器为每个客户端生成一个tcpsecket  

在客户端和服务端都定义了通信协议`protocol.h`  ，协议保持一致  



客户端：`QTcpSocket m_tcpSocket;`连接服务器，用户点击按钮发送信息，客户端对信息进行简单判断`on_send_pb_clicked()`，封装后发送给服务器。
- 连接服务器：`connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));`使用槽函数关联信号
- 发送数据：`m_tcpSocket.write()`直接用socket写过去

服务端：监听客户端的连接请求，生成socket（为每一个客户端生成一个独自的socket），信息处理。
- 监听客户端：设置ip和端口`MyTCPServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);`监听
- 处理请求：在mytcpserver.cpp中`void MyTCPServer::incomingConnection(qintptr socketDescriptor)`一旦连接就会触发，生成socket，并加入到list中
- 生成socket:自定义类MyTcpSocket，槽函数`connect(this, &QIODevice::readyRead, this, &MyTcpSocket::recvMsg);`各自的socket自己产生的消息，自己处理



8. 登录注册注销

- 数据库操作
```shell
F:\Qt\qt_project\TCPServer>sqlite3 cloud.db # 建库
SQLite version 3.43.2 2023-10-10 12:14:04
Enter ".help" for usage hints.
sqlite> create table userInfo(id integer primary key autoincrement, # 建用户表表：id主键自增
(x1...> name varchar(32) unique, # 用户名唯一
(x1...> pwd varchar(32), # 密码
(x1...> online integer default 0); # 是否在线：防止重复登录
sqlite> create table friend(id integer,friendId integer, # 建好友表
(x1...> primary key(id, friendId)); # 组合主键：自己id和朋友id
sqlite> .tables
friend    userInfo
sqlite> .quit

```



- 登录注册注销请求和回复
  1. 增加通信协议的消息类型：使用枚举
  2. 界面设计
  3. 注册：用户名唯一，防止重复注册
  4. 登录：防止重复登录
  5. 退出：将数据库中的在线状态设置为下线状态，并将socket删除
  6. 注销：删除好友关系，删除个人信息，删除网盘文件

```cpp
//客户端：编辑ui界面
void TCPClient::on_regist_pb_clicked()//按钮关联的事件
QMessageBox::critical(this,"注册","注册失败：用户名或密码不能为空！");//对用户输入进行简单的正则匹配，并输出提示信息
#include <QRegularExpression> //正则匹配需要的库

m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//把制造好的PDU发送给服务器

//服务器端：数据库操作
#include <QSqlDatabase> //连接数据库
#include <QSqlQuery> //查询数据库
class OperateDB : public QObject//自定义数据库操作类
void init();//连接化数据库
bool handleRegist(const char *name,const char *pwd);//处理注册请求
```



客户端和服务端的交互：
- 通过`socket`写出`write`数据，`read`读出数据
- 槽函数检测消息，有数据则根据消息类型（枚举类型）处理，`connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));//一旦服务器发送来数据，则接收，并处理`
- 有关数据库操作封装在了类中
- 收发数据，使用switch-case处理不同消息，制造pdu，发送pdu，定义一些宏，方便操作



9. 好友操作界面，文件操作界面     

整体界面：GUI编程，和 Java 的`spring`差不多，new组件，然后布局。   

只实现了好友操作界面，没有实现功能。



10. 功能实现

- **查看所有在线用户**：客户端发送查看请求 -> 服务器将数据库中在线的用户查询出来并发送给客户端（只发送用户名字） -> 客户端接收在线用户信息并显示

- **搜索用户**：客户端发送查看请求 -> 服务器将数据库中在用户查询（模糊查询）出来并发送给客户端（发送用户的名字，在线状态） -> 客户端接收用户信息并显示  *//客户端发送用户名，服务器查询，返回数据，客户接收数据*

> Qt运行是突然崩溃常见的几个原因：
>
> - 内存使用过多：QT应用程序在运行时消耗大量内存，如果已经超出系统可用内存，则会导致程序崩溃。
>
> - 程序中存在内存泄漏：内存泄漏通常发生在动态分配内存，并没有正确释放这些内存块，导致程序消耗的内存越来越大，最终导致程序崩溃。
>
> - 程序中存在空指针：如果代码中存在未初始化或者已经被删除的指针，则该指针所指向的内存地址为空，当程序试图访问这个空指针时，就会导致程序崩溃。
>
> - 多线程问题：QT支持多线程编程，在多线程编程中，如果没有正确同步和保护共享资源，则会导致竞态条件和死锁等问题，进而导致程序崩溃。
>
> - 库文件版本不匹配：QT库文件版本不匹配也可能导致程序崩溃。
>
>   解决这些问题需要仔细检查代码逻辑，进行调试和测试，以及对资源的合理管理与优化。

- **添加好友**：客户端发送添加b用户好友请求（发送信息包括双方的用户名）->服务器收到请求在数据库中查询（双方已是好友，用户不存在，用户不在线，用户在线）->若b用户在线，则服务器端向其发送添加好友请求->客户端b收到请求（同意或拒绝），向服务器发送数据->服务器接收结果，同意则修改数据库->向客户端反馈数据

```cpp
strcpy(resPdu->caData,REGIST_FAILED);//复制
strcmp(strFriendName.toStdString().c_str(),"所有在线用户如下：")//地址，比较
memcpy((char*)(respdu->caMsg)+i*32,nameList.at(i).toStdString().c_str(),nameList.at(i).size());

strcmp(pdu->caData,LOGIN_OK)//比较

QMessageBox::information(this,"登录",LOGIN_FAILED);//提示框

//插入好友关系
QString selectFridendId=QString("select id from userInfo where name=\'%1\'").arg(friendName);
QString selectMyId=QString("select id from userInfo where name=\'%1\'").arg(myName);
QString sqlStatement=QString("insert into friend (id,friendId) values ((%1),(%2))").arg(selectMyId).arg(selectFridendId);
qDebug()<<sqlStatement;

//解决叉掉就崩溃的问题
this->setAttribute(Qt::WA_DeleteOnClose);
```



```shell
# 数据库操作
F:\Qt\qt_project\TCPServer>sqlite3 ./cl
SQLite version 3.43.2  
Enter ".help" for usage hints.

sqlite> insert into friend (id,friendId) values(1,2),(4,8),(1,3),(3,2); # 插入
sqlite> select * from friend; # 查询
1|2
4|8
1|3
3|2
sqlite> select * from friend where id=(select id from userinfo where name='jack') and friendId=(select id from userinfo where name='rose');
1|2
sqlite> select * from friend where id =(select id from userinfo where name='cat') or friendId=(select id from userinfo where name='cat' );
1|3
3|2

sqlite> delete from friend; # 删除
sqlite> select * from friend;
sqlite> INSERT INTO friend (id, friendId)
   ...> VALUES ((SELECT id FROM userInfo WHERE name='jack'), (SELECT id FROM userInfo WHERE name='cat'));
sqlite> select * from friend;
1|3

sqlite> .schema userInfo # 得到表的详细信息
CREATE TABLE userInfo(id integer primary key autoincrement,
name varchar(32) unique,
pwd varchar(32),
online integer default 0);

sqlite> update userInfo set online=0; # 修改

sqlite> select * from userInfo;
1|jack|jack|0
2|rose|rose|0
3|cat|cat|0
4|crocuser|zaq1|0
8|lycher|zaq1|0
sqlite>
```

- **刷新好友列表**：客户端发送刷新好友列表请求（发送信息包括请求方用户名）->服务器将数据库中好友信息打包发送给客户端->客户端接收信息并显示

```shell
sqlite> select name,online from userInfo where id in (select friendId from friend where id in (select id from userInfo where name='jack')) or id in (select id from friend where friendId in (select id from userInfo where name='jack'));
rose|0
cat|0
crocuser|0
sqlite>
```

- **删除好友**：删除好友：客户端发送删除请求（发送信息包括双方用户名）->服务器将数据库中的好友信息删除->服务器回复客户端a和客户端b->客户端接收回复并显示  *//编写槽函数，关联信号槽。*
- **私聊**：私聊按钮和槽函数关联，显示私聊窗口，发送按钮制造pdu并发送。客户端a发送私聊消息请求（发送的信息包括双方的用户名、聊天信息），如果对方在线则转发成功，客户端b接收消息并显示私聊聊天框。
- **群聊**：客户端a发送群聊信息请求（发送的信息包括用户名和聊天信息）->服务器转发给所有在线好友->好友接收信息并显示。

  

11. 文件操作界面设计

  

12. 功能实现

- **新建目录或文件**：（注册新用户时发送用户名作为根目录）客户端发送查看请求（发送的信息包含用户名、目录信息、新建目录名）->服务器创建目录并发送反馈给用户->客户端接收反馈信息并显示
  - 注册成功后创建用户根目录
  - 登录成功后获取用户根目录

- **刷新目录（查看目录）**：展示当前路径下的所有文件，客户端发送查看目录请求（发送信息包含目录信息）->服务器将目录下所有文件名发给客户端->客户端接收回复信息并显示

- **进入目录**：双击条目进入该目录，主要是在客户端进行字符串拼接，得到所要进入查看的目录，然后发送刷新目录请求给服务端。

- **返回父目录**：点击返回按钮回到父目录，主要是在客户端进行字符串分割，得到所要返回的父目录路径，然后发送刷新目录请求给服务端。

- **删除文件或目录**：客户端发送删除请求（包含目录信息和要删除的文件名）->服务器删除文件并发送给客户端->客户端收到回复消息并显示

右键listwidget条目自定义菜单，将删除功能移至菜单，

遇到的问题是：如果使用自定义菜单，需要向编译器说明`this->m_pFileListW->setContextMenuPolicy(Qt::CustomContextMenu);//表示自定义上下文菜单策略。`

关联信号槽`connect(m_pFileListW, &QListWidget::customContextMenuRequested, this, &NetdiskFile::showContextMenu);//右键关联自定义菜单`

如果使用默认全局菜单：只用重写`contextMenuEvent(QContextMenuEvent *event) `，已经关联，自动调用

```cpp
//槽函数如下
void NetdiskFile::showContextMenu(const QPoint &pos) //添加菜单条目
{
    QPoint globalPos=this->m_pFileListW->mapToGlobal(pos);//从QListWidget的本地坐标pos转换为全局坐标，确保右键菜单在鼠标指针的正确位置显示
    QListWidgetItem *selectedItem=this->m_pFileListW->itemAt(pos);//将检查右键点击的位置是否在列表视图的项目范围内，返回指向被选中项目的指针。如果指针为nullptr，则意味着右键点击不是在任何项目上，因此右键菜单不应该显示。
    qDebug() << "Custom context menu requested!";
    //右键选择listwidget条目时，弹出菜单
    if(selectedItem)
    {
        QMenu *m_pMenu=new QMenu(this);
        QAction *pOpenACT=new QAction("打开",this);
        QAction *pDownloadACT=new QAction("下载",this);
        QAction *pShareACT=new QAction("分享",this);
        QAction *pCopyACT=new QAction("复制",this);
        QAction *pMoveACT=new QAction("移动",this);
        QAction *pDeleteACT=new QAction("删除",this);
        QAction *pCancelACT=new QAction("取消",this);

        connect(pDeleteACT,&QAction::triggered,this,&NetdiskFile::deleteFile);//关联槽函数
        //功能待完善

        m_pMenu->addAction(pOpenACT);
        m_pMenu->addAction(pDownloadACT);
        m_pMenu->addAction(pShareACT);
        m_pMenu->addAction(pCopyACT);
        m_pMenu->addAction(pMoveACT);
        m_pMenu->addAction(pDeleteACT);
        m_pMenu->addAction(pCancelACT);

        m_pMenu->exec(globalPos);
    }
}
```

- **重命名文件或目录**：客户端发送重名名请求（包含目录信息，要修改的文件名及新文件名）->服务器将文件重命名并回复客户端->客户端接收回复并显示
- **上传文件**：客户端发送上传请求（发送信息包括当前路径，上传文件名，上传文件内容）->服务器接收客户端文件数据到文件中，接收完后回复客户端->客户端接收回复信息并显示

上传文件内容使用二进制，为了防止数据粘连，设置了一个定时器。  

客户端上传文件的文件名，发送pdu

->服务器端接收pdu，创建一个网盘当前目录下对应的空文件，并保存文件的大小和文件路径（网盘中），设置当前服务器状态为接收二进制流状态，为后续接收文件内容做准备

->客户端设置计时器，等待服务器调整好状态->计时器到时，触发客户端传输文件数据流的槽函数，向服务器端发送数据流（每次4096，while循环）

->服务器端根据状态判断当前接收的数据类型，接收数据流，使用socket从QIODiver中全部读出，写入文件，通过字节数判断文件是否上传成功，发送上传文件回复pdu

->客户端接收pdu并显示回复消息

- **下载文件**：客户端发送下载请求（信息包括当前路径，下载文件名）->服务器提取指定文件内容及大小发送给客户端->客户端接收文件内容保存在文件中

服务器先告知客户端下载文件的大小，然后传输文件内容，否则客户端不知文件何时传输完毕。

- **分享文件**：分享界面设计->客户端发送分享文件请求（包含分享者、接收者（人名和个数）、当前路径、文件名）->服务器对请求进行操作，并将操作结果返回给客户端->客户端接收回复信息并显示
- **移动文件（剪切和粘贴）**：客户端发送移动请求（信息包含当前目录和目标目录）->服务器对文件进行移动操作，并回复客户端->客户端接收回复信息并显示

先剪切文件或目录，然后粘贴，只能粘贴一次。