
## 项目的技术要点
- 多线程 //网课中没有介绍如何使用多线程。。 
- TCP Socket网络编程
- SQLite3数据库
- 面向对象编程

采用C/S架构，数据存储用户信息，硬盘存储用户文件。


注：需要修改SQLite数据库文件的路径（绝对路径）

## 项目过程
### 1. 安装 SQLite

### 2. 设计数据表 

用户信息表
|字段|类型|约束条件|其他|
|--|--|--|--|
|id|integer|primary key|autoincrement|
|name|varchar(32)|not null||
|password|varchar(32)|not null||

用户好友表
|字段|类型|约束条件|其他|
|--|--|--|--|
|id|integer|主键|外键|
|friendId|integer|主键|外键|

```shell
PS F:\SQLite3\db> sqlite3 ./could.db # 建库
SQLite version 3.43.2 2023-10-10 12:14:04
Enter ".help" for usage hints.
sqlite> create table userInfo(id integer primary key autoincrement,
(x1...> name varchar(32),
(x1...> password varchar(32)); # 建表
sqlite> .tables # 查表
userInfo
sqlite> create table friendInfo(id integer not null,
(x1...> friendId integer not null,
(x1...> primary key(id , friendId));
sqlite> .tables
friendInfo  userInfo
sqlite> insert into userInfo(name,password) values('jack','jack'),
   ...> ('rose','rose'),('lucy','lucy');
sqlite> select * from userInfo # 查找所有用户信息
   ...> ;
1|jack|jack
2|rose|rose
3|lucy|lucy
sqlite> .quit # 退出
PS F:\SQLite3\db>
```

### 3. 安装Qt Creator
问题："No suitable kits found" = 没有找到合适的kits套件，在安装Qt Creator时没有安装MinGW，所以只需要进行安装即可。
解决方法：选择安装目录下的“MaintenanceTool.exe”，双击计入组件安装界面，根据自己安装的版本选择MinGW组件，点击下一步。重启后就会出现 kits。
快捷键：
- 函数声明和定义之间切换：ctrl+鼠标左键
- 返回：alt+←
- .h和.cpp之间切换：F4
- 自动生成函数定义：alt +enter

### 4. 配置文件的加载
- 在客户端代码目录下创建配置文件：IP，端口
- 作为资源文件添加入项目，添加前缀，添加文件
- 读取文件，处理数据`TCPClient::loadConfig()`

### 5. tcp客户端连接服务器
加载配置文件->产生socket->连接服务器（成功连接服务器会发出connected()信号，以此判断是否成功连接服务器）
->接收服务器数据
->发送数据给服务器
在TCPClient构造函数中调用`loadConfig()`加载配置文件，`connect()`关联信号和槽，`m_tcpSocket.connectToHost()`连接服务器

### 6. 服务器的实现
- 加载配置文件
- 连接数据库
- 接收客户端的连接

加载配置文件及打开数据库->QTcpServer监听listen（自定义类并继承`QTcpServer`，单例模式static）->循环接收客户端连接（服务器永不停机（？）->获取新QTcpSocket->与客户端数据交互及数据库操作
`MyTCPServer::getInstance().listen()`监听
有客户端连接上服务器后，服务器自动调用`incomingConnection()`

![alt text](imgs/image-2.png)
![alt text](imgs/image-3.png)

### 7. 通信协议设计
- 弹性结构体。//任何类型的指针，在64位系统中都占8个字节。
弹性数组

通信协议的设计(protocol data unity 协议数据单元)
```cpp
//协议结构体
struct PDU  //协议数据单元
{
    uint uiPDULen;  //总的协议数据单元大小,ui无符号整型
    uint uiMsgType; //消息类型，如注册，登录，加好友
    char caData[64]; //用户名、密码
    uint uiMsgLen; //实际消息长度
    int caMsg[]; //实际消息
};
PDU *mkPDU(uint uiMsgLen);
```
*在Qt中，资源文件（由 .qrc 文件定义）被编译到应用程序的可执行文件中，它们在运行时被当作只读数据对待。这意味着你不能直接修改这些资源文件的内容，因为它们被嵌入在可执行文件中，任何尝试写入这些资源文件的操作都会导致运行时错误。*

收发数据
服务器为每个客户端生成一个tcpsecket，服务器自定义`MyTcpSocket`类继承自`QTcpSocket`。
在客户端和服务端都定义了通信协议`protocol.h`

客户端：`QTcpSocket m_tcpSocket;`连接服务器，用户点击按钮发送信息，客户端对信息进行简单判断`on_send_pb_clicked()`，封装后发送给服务器。
- 连接服务器：`connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));`使用槽函数关联信号
- 发送数据：`m_tcpSocket.write()`直接用socket写过去

服务端：监听客户端的连接请求，生成socket（为每一个客户端生成一个独自的socket），信息处理。
- 监听客户端：设置ip和端口`MyTCPServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);`监听
- 处理请求：在mytcpserver.cpp中`void MyTCPServer::incomingConnection(qintptr socketDescriptor)`一旦连接就会触发，生成socket，并加入到list中
- 生成socket:自定义类MyTcpSocket，槽函数`connect(this, &QIODevice::readyRead, this, &MyTcpSocket::recvMsg);`各自的socket自己产生的消息，自己处理
- 然后就可以使用socket进行数据收发了。

### 8. 登录注册注销
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
- 定义数据库操作类，使用单例模式

```cpp
//服务端
#include <QSqlDatabase> //连接数据库
#include <QSqlQuery> //查询数据库
```

- 登录注册注销请求和回复

1. 增加通信协议的消息类型：使用枚举
2. 界面设计
3. 注册：用户名唯一，防止重复注册
4. 登录：记录用户名和在线状态，防止重复登录
5. 退出：根据用户名将数据库中的在线状态设置为下线状态，并将list中的socket删除
6. 注销：删除好友关系，删除个人信息，删除网盘文件

在注册登录时，添加了哈希加密算法，提高用户的隐私安全性。

流程：
用户点击了ui上的按钮，客户端经过检查，向服务器发送请求PDU，
服务器监听，接收到了客户端的请求，并进行相应处理，数据库操作和文件操作等，向客户端发送处理结果，即相应PDU，
客户端接收响应，向用户展示操作结果。

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

//注意执行的sql语句是否正确(细节)

```


客户端和服务端的交互：
- 通过`socket`写出`write`数据，`read`读出数据
- 槽函数检测消息，有数据则根据消息类型（枚举类型）处理，`connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));//一旦服务器发送来数据，则接收，并处理`
- 有关数据库操作封装在了类中
- 收发数据，使用switch-case处理不同消息，制造pdu，发送pdu，定义一些宏，方便操作


在Qt中，“信号”是一种特殊的函数，它是一种用于对象间通信的方式。
信号只需声明，而不需要实现。当某个事件在对象中发生时，该对象将发射（emit）一个信号，通知其他对象或者系统某个事件已经发生了。
其他对象可以连接（connect）到该信号，以响应事件并执行相应的操作。

（tcpsocket向tcpserver发送信号）用户状态的更改：关闭窗口--下线（更改状态，删除socket）
自定义信号，当更改在线状态时，也连同删除socket
```cpp
//定义信号
signals:
   void offline(MyTcpSocket *mysocket);

//槽函数
public slots:
   void clientOffline();
//关联
connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deleteSocket(MyTcpSocket*)));

//发出信号
void MyTcpSocket::clientOffline()
{
    //设置在线状态
    OperateDB::getInstance().handleOffline(m_strName.toStdString().c_str()); //参数为char*
    emit offline(this);//this 该对象的地址
}

//处理信号
void MyTCPServer::deleteSocket(MyTcpSocket *mySocket)
{
    //遍历list，然后删除
    QList<MyTcpSocket*>::iterator iter=m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();++iter)
    {
        if(mySocket==*iter)
        {
            delete *iter;//new的对象删除
            *iter=NULL;
            m_tcpSocketList.erase(iter);//指针删除
            break;
        }
    }
    for(int i=0;i<m_tcpSocketList.size();++i)
    {
        qDebug()<<m_tcpSocketList.at(i)->getName();    }
}
```


### 9. 好友操作界面UI

操作界面：operatewidget.cpp--QListWidget(列表窗口)、QStackedWidget(堆栈窗口)、`connect(m_pListW,SIGNAL(currentRowChanged(int)),m_pStackedW,SLOT(setCurrentIndex(int)));`两个窗口关联，堆栈窗口中有好友窗口和文件窗口。
所有在线好友界面：allonline.ui
好友界面：MyFriendList.cpp
文件界面：netdiskfile.cpp


好友操作：
- 聊天
- 搜索好友
所有在线好友，搜索好友，添加好友

整体界面：GUI编程，和 Java 的`spring`差不多，new组件，然后布局。
只实现了界面，没有是实现功能。

### 10. 功能实现
登录跳转，成功登录后，显示操作界面，隐藏登录界面。

#### 查看在线用户
- 客户端发送查看请求 -> 服务器将数据库中在线的用户查询出来并发送给客户端（只发送用户名字） -> 客户端接收在线用户信息并显示

```cpp
public:
    //单例模式
    static TCPClient &getInstance();
    //使用tcpsocket发送请求，同一个对象，其他类也可以使用
    QTcpSocket &getTcpSocket();//这个是私有属性

15:54:08: F:\Qt\qt_project\build-TCPServer-Desktop_Qt_6_3_2_MinGW_64_bit-Debug\debug\TCPServer.exe 崩溃。
这个服务器崩溃的出现是在我关了第二个操作界面后，有点问题。
再次点击查看在线用户时，客户端崩溃。。。解决：点击叉号后不能删除窗口，应当隐藏（默认处理即可）

//显示服务器响应中的所有在线用户
void TCPClient::recvMsg()
{
    ……
    case ENUM_MSG_ALL_ONLINE_RESPOND://所有在线用户回复
        OperateWidget::getInstance().getFriendList()->showAllOnlineUser(pdu);
    ……
}

void MyFriendList::showAllOnlineUser(PDU *pdu)
{
    if(NULL==pdu) return;
    m_pAllOnline->showUser(pdu);
}
//tcpClient接收响应，调用操作窗口中的好友窗口去显示所有在线用户
//好友窗口调用自身的子窗口显示所有在线用户
```

```shell
sqlite> .schema userInfo # 得到表的详细信息，这不就是建表语句吗
CREATE TABLE userInfo(id integer primary key autoincrement,
name varchar(32) unique,
pwd varchar(32),
online integer default 0);

```

#### 搜索用户
- 客户端发送查看请求 -> 服务器将数据库中在用户查询出来并发送给客户端（发送用户的名字，在线状态） -> 客户端接收用户信息并显示

客户端发送用户名，服务器查询，返回数据，客户接收数据

> Qt运行是突然崩溃常见的几个原因：
> 内存使用过多：QT应用程序在运行时消耗大量内存，如果已经超出系统可用内存，则会导致程序崩溃。
> 程序中存在内存泄漏：内存泄漏通常发生在动态分配内存，并没有正确释放这些内存块，导致程序消耗的内存越来越大，最终导致程序崩溃。
> 程序中存在空指针：如果代码中存在未初始化或者已经被删除的指针，则该指针所指向的内存地址为空，当程序试图访问这个空指针时，就会导致程序崩溃。
> 多线程问题：QT支持多线程编程，在多线程编程中，如果没有正确同步和保护共享资源，则会导致竞态条件和死锁等问题，进而导致程序崩溃。
> 库文件版本不匹配：QT库文件版本不匹配也可能导致程序崩溃。
解决这些问题需要仔细检查代码逻辑，进行调试和测试，以及对资源的合理管理与优化。

```cpp
OperateWidget &OperateWidget::getInstance()
{
    static OperateWidget instance;
    instance.setAttribute(Qt::WA_DeleteOnClose);//叉掉自动删除
    return instance;
}
```

//本来想把搜索结果展示的可以添加好友的那个界面上，但是吧，数据传过去客户端就崩溃，最后，新建了个窗口，显示模糊查询结果。。。

#### 添加好友

控件转到槽，会生成槽函数的声明和定义，且自动关联信号和槽。

```cpp
//总是忘记如何写。。
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

```

```shell
# 数据库操作
sqlite> insert into friend (id,friendId) values(1,2),(4,8),(1,3),(3,2);
sqlite> select * from friend where id=(select id from userinfo where name='jack') and friendId=(select id from userinfo where name='rose');
sqlite> delete from friend;
```

添加好友：客户端发送请求（发送信息包括双方的用户名）->服务器收到请求在数据库中查询（双方已是好友，用户不存在，用户不在线，用户在线）
->若b用户在线，则服务器端向其发送添加好友请求
->客户端b收到请求（同意或拒绝），向服务器发送数据
->服务器接收结果，同意则修改数据库->向客户端反馈数据


#### 刷新好友列表

刷新好友：客户端发送刷新好友列表请求（发送信息包括请求方用户名）->服务器将数据库中好友信息打包发送给客户端->客户端接收信息并显示

```shell
sqlite> select name,online from userInfo where id in (select friendId from friend where id in (select id from userInfo where name='jack')) or id in (select id from friend where friendId in (select id from userInfo where name='jack'));
rose|0
cat|0
crocuser|0
```

用 free 释放由 malloc 分配的内存，用 delete 释放由 new 分配的内存.

#### 删除好友

删除好友：客户端发送删除请求（发送信息包括双方用户名）->服务器将数据库中的好友信息删除->服务器回复客户端a和客户端b->客户端接收回复并显示

编写槽函数，关联信号槽。

#### 私聊

创建私聊窗口。
私聊按钮和槽函数关联，显示私聊窗口，发送按钮制造pdu并发送。

客户端a发送私聊消息请求（发送的信息包括双方的用户名、聊天信息），如果对方在线则转发成功，客户端b接收消息并显示
```cpp
//防中文乱码
QByteArray byteArray = strMessage.toUtf8();
```
给私聊窗口设置标题，以及添加发送消息的时间，发送方也可以看到自己发送的消息。

#### 群聊

客户端a发送群聊信息请求（发送的信息包括用户名和聊天信息）->服务器转发给所有在线好友->好友接收信息并显示
//在线群聊之sql总写错，and没有将查询结果加到list中。。。

### 11. 文件操作UI
- 界面设计

### 12.  功能实现

#### 新建目录
前提：
（1）注册成功后创建用户根目录
（2）登录成功后获取用户根目录

客户端发送新建目录请求（发送的信息包含用户名、目录信息、新建目录名）->服务器创建目录并发送反馈给用户->客户端接收反馈信息并显示

//注意：目录名称只能输入英文字符。

#### 刷新目录（查看目录）
- 展示当前路径下的所有文件

客户端发送查看目录请求（发送信息包含目录信息）->服务器将目录下所有文件名发给客户端->客户端接收回复信息并显示

```cpp
//编码的问题，导致传输数据不完整，显示不全
QByteArray byteArray = message.toUtf8();//将 QString 转换为 UTF-8 编码格式
memcpy((char*)(respdu->caMsg)+i*128,byteArray,byteArray.size());
qDebug()<<byteArray;
```

### 进入目录，退出目录
双击目录条目，进入该目录，
点击返回按钮回到父目录。
```cpp
    int index=strPath.lastIndexOf('/');//找最后一个/，然后删除后面的
    strPath.remove(index,strPath.size()-index);
```
在客户端接收用户想要查看的目录，调用刷新目录的函数，来显示服务器返回的目录信息即可。
只需写客户端的槽函数，向服务器发送信息（包括目录路径），消息类型都是刷新目录请求，服务器同意查询该路径下的文件信息，反馈给客户端，客户端接收并显示。

//双击空白处，退出文件夹****还没想到解决方法。

#### 删除文件，删除目录

客户端发送删除请求（包含目录信息和要删除的文件名）->服务器删除文件并发送给客户端->客户端收到回复消息并显示

本人实现了：右键listwidget条目自定义菜单，将删除功能移至右键菜单，
遇到的问题是：如果使用自定义菜单，需要向编译器说明`this->m_pFileListW->setContextMenuPolicy(Qt::CustomContextMenu);//表示自定义上下文菜单策略。`
关联信号槽`connect(m_pFileListW, &QListWidget::customContextMenuRequested, this, &NetdiskFile::showContextMenu);//右键关联自定义菜单`

如果使用默认全局菜单：只用重写`contextMenuEvent(QContextMenuEvent *event) `，已经关联，自动调用

```cpp
//槽函数如下
void NetdiskFile::showContextMenu(const QPoint &pos) //添加菜单条目
{
    QPoint globalPos=this->m_pFileListW->mapToGlobal(pos);//从QListWidget的本地坐标pos转换为全局坐标，确保右键菜单在鼠标指针的正确位置显示
    QListWidgetItem *selectedItem=this->m_pFileListW->itemAt(pos);//将检查右键点击的位置是否在列表视图的项目范围内，返回指向被选中项目的指针。如果指针为nullptr，则意味着右键点击不是在任何项目上
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
#### 新建文件
客户端发送新建文件请求->服务器接收路径和文件名，创建文件，封装回复的PDU->客户端接收回复信息并显示

//问题：路径中只能是英文字符，不然创建失败，即中文乱码

#### 重命名文件

客户端发送重名名请求（包含目录信息，要修改的文件名及新文件名）->服务器将文件重命名并回复客户端->客户端接收回复并显示
重命名文件或目录菜单项功能已实现。
实现：右键文件列表空白处，新建文件

#### 上传文件

客户端发送上传请求（发送信息包括当前路径，上传文件名，上传文件内容）->服务器接收客户端文件数据到文件中，接收完后回复客户端->客户端接收回复信息并显示

上传文件内容使用二进制，为了防止数据粘连，设置了一个定时器。

客户端上传文件的文件名，发送pdu
->服务器端接收pdu，创建一个网盘当前目录下对应的空文件，并保存文件的大小和文件路径（网盘中），设置当前服务器状态为接收二进制流状态，为后续接收文件内容做准备
->客户端设置计时器，等待服务器调整好状态->计时器到时，触发客户端传输文件数据流的槽函数，向服务器端发送数据流（每次4096，while循环）
->服务器端根据状态判断当前接收的数据类型，接收数据流，使用socket从QIODiver中全部读出，写入文件，通过字节数判断文件是否上传成功，发送上传文件回复pdu
->客户端接收pdu并显示回复消息

```cpp
this->m_strUploadFile;//问题：一次只能上传一个文件，多个文件会造成混乱
if (this->m_bIsDataStream)//如果是数据流
{
    this->recvDataStream();//调用数据流进行处理
    return;
}
QByteArray buffer=this->readAll();//一直读取数据，直到设备没有更多的数据可供读取，或者发生了错误。
```

问题：
1. 客户端和服务器端应该都分块收发数据，可以加入多线程，提高效率，
2. 客户端出现异常的情况没有考虑
3. 加个上传过程中的进度条，更美观


#### 下载文件

客户端发送下载请求（信息包括网盘当前路径，下载文件名）->服务器提取指定文件内容及大小发送给客户端->客户端接收文件内容保存在文件中

服务器先告知客户端下载文件的大小，然后传输文件内容，否则客户端不知文件何时传输完毕。

上传文件和下载文件类似：
- 上传文件：
  - 客户端，上传文件名和文件大小，设置定时器->
  - 服务器端接收文件名和文件大小后，设置接收数据流的状态，打开文件->
  - 定时器到时，停止定时器，客户端开始循环发送数据流->
  - 服务器端接收数据流写入文件，当接收的数据流大小大于等于文件大小后，停止接收数据流，设置接收数据流的状态，封装接收结果的PDU->
  - 客户端接收回复并显示信息
- 下载文件：
  - 客户端，选择要下载的文件，选择下载的位置->
  - 服务器返回文件名和文件大小，设置定时器->
  - 客户端接收文件名和文件大小，设置接收数据流的状态，打开文件->
  - 定时器到时，停止定时器，服务器端开始循环发送数据流->
  - 客户端接收数据流写入文件，当接收的数据流大小大于等于文件大小后，停止接收数据流，设置接收数据流的状态，显示下载是否成功的信息


#### 分享文件

分享界面设计->客户端发送分享文件请求（包含分享者、接收者（人名和个数）、当前路径、文件名）->服务器对请求进行操作，并将操作结果返回给客户端->客户端接收回复信息并显示



#### 移动文件（剪切和粘贴）

客户端发送移动请求（信息包含当前目录和目标目录）->服务器对文件进行移动操作，并回复客户端->客户端接收回复信息并显示

剪切，然后粘贴，只能粘贴一次（粘贴时清空保存的剪切路径，有点简单粗暴），可以是文件也可以是目录。


//一些功能未实现

