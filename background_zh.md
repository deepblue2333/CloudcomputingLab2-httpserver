## 2. 背景

### 2.1 超文本传输协议

超文本传输协议（HTTP）是当今互联网上最常用的应用协议。

与许多网络协议一样，HTTP使用客户端-服务器模型。HTTP客户端打开到HTTP服务器的网络连接并发送HTTP请求消息。然后，服务器用HTTP响应消息回复，通常包含客户端请求的某些资源（文件、文本、二进制数据）。

我们在本节中简要介绍HTTP消息的格式和结构，以方便您理解。HTTP/1.1的详细规范可以在[RFC 2616 - 超文本传输协议 -- HTTP/1.1](https://tools.ietf.org/html/rfc2616)中找到。

### 2.2 HTTP消息

HTTP消息是简单的、格式化的数据块。

所有HTTP消息分为两种类型：**请求**消息和**响应**消息。

- 请求消息请求服务器执行一个动作。

- 响应消息将请求的结果返回给客户端。

请求和响应消息都具有相同的基本消息结构。

#### 2.2.1 消息格式

HTTP请求和响应消息由3个组件组成：

- 描述消息的起始行，
- 包含属性的头部块，
- 可选的包含数据的主体。

每个组件的格式如下所示

##### 2.2.1.1 起始行

所有HTTP消息都以起始行开头。请求消息的起始行说明*要执行什么操作*。响应消息的起始行说明*发生了什么*。

具体来说，起始行在*请求消息*中也称为***请求行***，在*响应消息*中称为***响应行***。

- **请求行**
   - 包含描述服务器应执行的操作以及描述要在其上执行操作的资源的请求URL的方法。
   - 还包括一个HTTP版本，告诉服务器客户端所使用的HTTP方言。所有这些字段都由空格分隔。
   - 例如 `GET /index.html HTTP/1.1`

- **响应行**
  - 包含响应消息正在使用的HTTP版本、数字状态码和描述操作状态的文本原因短语。
  - 例如 `HTTP/1.1 200 OK`

##### 2.2.1.2 头部

在起始行之后是零个、一个或多个HTTP头部字段的列表。

HTTP头部字段为请求和响应消息添加了附加信息。它们基本上只是名称/值对的列表。

每个HTTP头部都具有简单的语法：名称，后跟冒号`:`，后跟可选的空白，后跟字段值，后跟一个`CRLF`。

HTTP头部被分类为：
 * 通用头部
 * 请求头部
 * 响应头部
 * 实体头部 
 * 扩展头部

请注意，请求头部字段与响应头部字段是不同的。我们不会详细介绍这些字段，您无需在此实验中实现它们。

您可以在[RFC 2616 - 超文本传输协议 -- HTTP/1.1](https://tools.ietf.org/html/rfc2616)中找到它们。

请求中头部的示例：

```
Host: 127.0.0.1:8888
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:74.0) Gecko/20100101 Firefox/74.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Cache-Control: max-age=0
```

响应中头部的示例：

```
Server: Guo's Web Server
Content-length: 248
Content-type: text/html
```

##### 2.2.1.3 实体主体

HTTP消息的第三部分是可选的实体主体。实体主体是HTTP消息的有效载荷。它们是HTTP旨在传输的内容。

HTTP消息可以携带许多种类的数字数据：图像、视频、HTML文档、软件应用程序、信用卡交易、电子邮件等等。

实体主体的示例：

```
<html><head>
<title>CS06142</title>
</head><body>
<h1>CS06142</h1>
<p>Welcome to Cloud Computing Course.<br />
</p>
<hr>
<address>Http Server at ip-127-0-0-1 Port 8888</address>
</body></html>
```

#### 2.2.2 HTTP请求的结构

HTTP请求消息包含：

* 一个HTTP请求行：
  * 方法，
  * 查询字符串，
  * HTTP协议版本，
* 零个或多个HTTP头部行，
* 一个空行（即单独的`CRLF`），
* 请求需要携带的可选内容。

HTTP请求消息的示例：

```
GET /index.html HTTP/1.1   
Host: 127.0.0.1:8888
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:74.0) Gecko/20100101 Firefox/74.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Cache-Control: max-age=0
```

## 2.2.3 HTTP响应的结构

一个HTTP响应消息包含：

* HTTP响应状态行：
  * HTTP协议版本，
  * 状态码
  * 状态码的描述，
* 零个或多个HTTP头部行，
* 一个空行（即单独的`CRLF`） 
* HTTP请求所请求的内容。

HTTP响应消息的示例：

```
HTTP/1.1 200 OK  					
Server: Tiny Web Server
Content-length: 248
Content-type: text/html
									// CRLF
<html><head>
<title>CS06142</title>
</head><body>
<h1>CS06142</h1>
<p>Welcome to Cloud Computing Course.<br />
</p>
<hr>
<address>Http Server at ip-127-0-0-1 Port 8888</address>
</body></html>
```

`Host`请求头部指定了请求要发送到的服务器的主机和端口号。如果不包括端口，则隐含请求的服务的默认端口（例如，HTTPS URL的默认端口为443，HTTP URL的默认端口为80）。

`Content-Length`头部指示发送到接收者的消息主体的大小，以字节为单位。

`Content-Type`表示头部用于指示资源的原始媒体类型。

以下是一些示例：

| 文件扩展名 | MIME类型 |
| --- | --- |
| *.html | text/html |
| *.js | text/javascript |
| *.css| text/css |
| *.txt | text/plain |
| *.json | application/json |
| *.png | image/png |
| *.jpg | image/jpg |

### 2.3 HTTP代理

HTTP代理服务器是中间人。代理位于客户端和服务器之间，充当“中间人”，在各方之间来回传递HTTP消息。

HTTP代理服务器是以客户端的名义执行交易的中间人。没有HTTP代理，HTTP客户端直接与HTTP服务器通信。有了HTTP代理，客户端则与代理通信，代理本身与服务器通信。

HTTP代理服务器既是Web服务器又是Web客户端。由于HTTP客户端将请求消息发送到代理，因此代理服务器必须像Web服务器一样正确处理请求和连接并返回响应。同时，代理本身也会向服务器发送请求，因此它也必须像正确的HTTP客户端一样行为，发送请求并接收响应。

HTTP代理的工作模式如下图所示：

```
                               +-----------+               +-----------+
                               |           |               |           |
   +----------+    请求       |           |   请求       |           |
   |          |+--------------->           |+-------------->           |
   |  客户端  |                |   代理   |               |   服务器  |
   |          <---------------+|           <--------------+|           |          
   +----------+	   响应    |           |   响应       |           |          
                               |           |               |           |
                               +-----------+               +-----------+
```

## 2.4 JSON

JSON（JavaScript对象表示法）是一种轻量级的数据交换格式。它易于人类阅读和编写，也易于机器解析和生成。它基于JavaScript编程语言标准ECMA-262第3版 - 1999年12月的一个子集。JSON是一种完全与语言无关的文本格式，但使用了熟悉的C语言系列（包括C、C ++、C＃、Java、JavaScript、Perl、Python等）的约定。这些属性使JSON成为理想的数据交换语言。

JSON基于两种结构：

* 一组名称/值对。在各种语言中，这被实现为对象、记录、结构、字典、哈希表、键控列表或关联数组。

* 一系列值的有序列表。在大多数语言中，这被实现为数组、向量、列表或序列。

这些是通用的数据结构。几乎所有现代编程语言都以一种形式或另一种形式支持它们。一个可以与编程语言互换的数据格式也基于这些结构。

在JSON中，它们采用以下形式：

* 对象是一组无序的名称/值对。对象以`{`开始，以`}`结束。每个名称后跟`:`，名称/值对由`,`分隔。

* 数组是一组有序的值。数组以`[`开始，以`]`结束。值由`,`分隔。

* 值可以是双引号括起来的字符串，或数字，或true或false或null，或对象或数组。这些结构可以嵌套。

* 字符串是由双引号括起来的零个或多个Unicode字符序列，使用反斜杠转义。一个字符表示为单个字符字符串。字符串非常类似于C或Java字符串。

* 数字非常类似于C或Java数字，只是不使用八进制和十六进制格式。

* 可以在任何标记对之间插入空格。除了一些编码细节，这完全描述了语言。

例如：

```json
{
    "status": {
        "code": 200,
        "text": "OK"
    },
    "data" : [
        {
            "id": "S1",
            "name": "Foo"
        },
        "Bar\r\n",
        5.0,
        true,
        null
    ]
}
```

## 2.5 CURL

`curl`是一个工具，用于使用支持的协议之一（DICT、FILE、FTP、FTPS、GOPHER、HTTP、HTTPS、IMAP、IMAPS、LDAP、LDAPS、MQTT、POP3、POP3S、RTMP、RTMPS、RTSP、SCP、SFTP、SMB、SMBS、SMTP、SMTPS、TELNET和TFTP）从服务器传输数据。该命令旨在无需用户交互即可工作。

`curl`提供了大量实用的技巧，如代理支持、用户身份验证、FTP上传、HTTP POST、SSL连接、cookies、文件传输续传、Metalink等等。如下所示，您将看到的功能数量会让您眼花缭乱！

`curl`通过libcurl提供了所有与传输相关的功能。有关详细信息，请参阅libcurl(3)。

### 2.5.1 源代码

您可以从GitHub上查看最新的[源代码](https://github.com/curl/curl)。

### 2.5.2 安装

在许多操作系统上，默认情况下已经安装了curl。如果没有安装，您可以使用系统的软件包管理工具进行安装。例如，对于Ubuntu，可以使用`sudo apt install curl`命令进行安装。

### 2.5.3 教程

有关更多教程，请查看[curl教程](https://curl.se/docs/manual.html)或在安装后使用`man curl`命令。

这里我们提供了您在实验中可能需要的一些示例。

#### 2.5.3.1 GET

##### 访问静态资源

```shell
curl -i -X GET http://localhost:8080/index.html
```

##### 访问Web服务

```shell
curl -i -X GET http://localhost:8080/api/search
```

###### 带查询字符串

```shell
curl -i -G -d 'id=1&name=Foo' -X GET http://localhost:8080/api/search
```

##### 2.5.3.2 POST

##### 发送类似html <form />的数据

```shell
curl -i -d 'id=1&name=Foo'
    -H 'Content-Type: application/x-www-form-urlencoded'
    -X POST http://localhost:8080/api/upload
```

##### 使用form-data发送数据

```shell
curl -i -F "id=1" -F "name=Foo"
    -X POST http://localhost:8080/api/upload
```
##### 使用json发送数据

```shell
curl -i -d '{"id":"1","name":"Foo"}'
    -H 'Content-Type: application/json'
    -X POST http://localhost:8080/api/upload
```