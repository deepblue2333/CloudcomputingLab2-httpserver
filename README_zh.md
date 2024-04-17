# 实验2：自己的HTTP服务器

*部分材料来自加州大学伯克利分校2019年CS162课程的Homework 2。感谢CS162！*

进入您从我们的实验git仓库克隆的文件夹，并拉取最新的提交 - 使用`git pull`。

您可以在`Lab2/README.md`中找到此Lab2的说明。

Lab2的所有材料都在文件夹`Lab2/`中。

# 1. 概述

基于HTTP/1.1从头开始实现一个HTTP服务器，使用从我们课程中学到的网络编程知识。

同时，尝试使用从课程中学到的高并发编程技能来保证Web服务器的性能。

**我们的目标：**

* 练习基本的网络编程技能，如使用套接字API，解析数据包;
* 熟悉健壮且高性能的并发编程。

# 2. 背景

**请首先查看[background.md](./background.md)，了解有关`HTTP`、`HTTP消息`、`HTTP代理`、`JSON`和`curl`的基础知识。**

# 3. 实验任务

## 实现自己的HTTP服务器

在此实验中，我们不会提供任何基本代码。您应该根据HTTP/1.1从头开始实现一个HTTP服务器，满足以下要求：

**HTTP服务器概要**

从网络的角度来看，您的HTTP服务器应实现以下功能：

1. 创建一个监听套接字并将其绑定到一个端口;
2. 等待客户端连接到端口;
3. 接受客户端并获取新的连接套接字;
4. 读取并解析HTTP请求;
5. 开始提供服务（某些是可选的）：
   * 处理HTTP GET/POST请求并返回响应。
   * 代理请求到另一个HTTP服务器。

服务器将处于非代理模式或代理模式（我们在背景部分`2.3`中介绍了代理）。它不会同时执行这两个任务。

为了更好地进行测试和评分，我们为您提交的作品制定了**一些功能要求**。

**注意：Lab 2是Lab 4的前期实验。如果您计划完成Lab 4，请完成Lab 2的**高级版本**。**

### 3.1 处理HTTP请求并发送HTTP响应

在此Lab中，**您只需在您的HTTP服务器中实现GET方法和POST方法**。

对于任何其他方法，您的服务器应该返回状态码为501的响应（见`2.2`）。
也就是说，如果您的HTTP服务器接收到一个HTTP请求，但请求方法既不是GET也不是POST，那么HTTP服务器只需返回一个501 Not Implemented错误消息（响应消息的响应行带有状态代码为501，见`2.2`）。

请参见第[3.7](#37-access-your-http-server)节中的示例。

#### 3.1.1 处理HTTP GET请求

HTTP服务器应该能够处理特定资源的HTTP GET请求，例如Web服务数据和静态文件。

对于GET请求，服务器需要检查请求的路径是否对应于Web服务或存在的静态文件。

您需要执行以下操作：

**基本版本**：

* 搜索服务：使用特定的url从服务器获取一些不来自文件系统的数据。
  * url没有查询字符串，例如`http://localhost:8080/api/check`；
  * 以纯文本内容发送响应。

* 静态文件访问：使用特定的url访问存储在文件系统上的文本文件。
  * 客户端仅请求`*.html`；
  * 客户端可能在子目录中请求文件，例如`http://localhost:8080/test/index.html`；
  * 以html文件的完整内容发送响应。

* 如果路径无效或静态文件不存在
  * 以`404 Not Found`页面的完整内容发送响应。

**高级版本**：

* 搜索服务：使用特定的url从服务器获取一些不来自文件系统的数据。
  * url可能有查询字符串，例如`http://localhost:8080/search?id=1&name=foo`；
  * 查询字符串包含一些键值对。键是`id`和`name`；
  * 如果查询字符串有效，以json类型的内容发送响应。
  * 或者如果无效，以json类型的内容发送响应，包括错误消息。

* 静态文件访问：使用特定的url访问存储在文件系统上的文本文件。
  * 客户端可能请求`*.html`，`*.js`，`*.css`，`*.json`和其他纯文本文件；
  * 客户端可能在子目录中请求文件，例如`http://localhost:8080/test/index.html`；
  * 以正确的`Content-Type`和文件的完整内容发送响应。

* 如果路径无效或静态文件不存在
  * 以`404 Not Found`页面的完整内容发送响应。

*您不需要实现传输Base64编码的二进制文件，例如像`*.png`这样的图像文件。*

#### 3.1.2 处理HTTP POST请求

HTTP服务器应该能够处理HTTP POST请求。

对于POST请求，服务器需要检查请求的路径是否对应于Web服务。

> 对于特殊字符，如`%`，您可能需要查询有关**百分号编码**的信息。

您需要执行以下操作：

**基本版本**：

* 上传服务：使用特定的url向服务器上传一些数据
  * 内容类型为`application/x-www-form-urlencoded`；
  * 负载包含键值对，键是`id`和`name`；
  * 如果有效负载，则以`200 OK`、`Content-Type: text/plain`和数据发送响应；
  * 或者如果负载无效，则以`404 Not Found`、`Content-Type: text/plain`和错误消息发送响应；

*

 如果路径无效
  * 以`404 Not Found`页面的完整内容发送响应。

**高级版本**：

* 上传服务：使用特定的url从服务器上传一些数据，这些数据不来自文件系统。
  * 内容类型为`application/x-www-form-urlencoded`，`application/json`；
  * 负载包含键值对，键是`id`和`name`；
  * 如果有效负载，则以`200 OK`、`Content-Type: application/json`和数据发送响应；
  * 或者如果负载无效，则以`404 Not Found`、`Content-Type: application/json`和错误消息发送响应；

* 如果路径无效
  * 以`404 Not Found`页面的完整内容发送响应。

如果您不熟悉`application/x-www-form-urlencoded`，请查阅[MDN文档](https://developer.mozilla.org/en-US/docs/Learn/Forms/Sending_and_retrieving_form_data)。

#### 3.1.3 其他请求

对于除GET请求和POST请求之外的HTTP请求，只需返回501 Not Implemented页面。

### 3.2 实现代理服务器（高级版本可选）

使您的服务器能够代理HTTP请求到另一个HTTP服务器，并将响应转发到客户端。

1. 您应该使用`--proxy`命令行参数的值，其中包含上游HTTP服务器的地址和端口号。
   
2. 您的代理服务器应等待两个套接字上的新数据（HTTP客户端文件描述符和上游HTTP服务器文件描述符）。当数据到达时，您应立即将其读取到缓冲区中，然后将其写入另一个套接字。您实际上是在HTTP客户端和上游HTTP服务器之间维护双向通信。请注意，您的代理必须支持多个请求/响应。

3. 如果其中一个套接字关闭，通信无法继续，因此您应关闭另一个套接字并退出子进程。

提示：1）这比向文件写入或从标准输入读取更棘手，因为您不知道双向流的哪一侧将首先写入数据，或者在接收响应后是否会继续写入更多数据。2）您应再次使用线程来完成此任务。例如，考虑使用两个线程促进双向通信，一个从A到B，另一个从B到A。

> 大多数网站现在使用HTTPS并将检查您的HTTP标头行，因此您的代理服务器可能不适用于任何Web服务器。
> 
> 我们的任务不是实现除HTTP以外的其他协议。
> 
> 它们通常涉及加密算法，直接使用套接字API难以直接实现。
> 
> 但是，如果您对HTTPS等协议感兴趣，可以尝试使用其他库，例如openssl。

### 3.3 使用多线程提高并发性

您的HTTP服务器应使用多个线程处理尽可能多的并发客户端请求。您至少有以下三种选项来设计多线程服务器：

- **按需线程**。 每当新的客户端出现时，您可以创建一个新线程，并使用该线程处理所有该客户端的任务，包括解析HTTP请求、获取页面文件和发送响应。然后，在该客户端完成后销毁线程，例如，通过TCP `recv()`检测到。

- **始终开启的线程池**。 您可以在HTTP服务器程序中使用固定大小的线程池来处理多个客户端请求。如果没有任务，这些线程处于等待状态。如果有新的客户端进入，将线程分配给处理客户端请求并向其发送响应。如果分配的线程正忙，您可以使用工作队列来缓冲请求，让线程稍后处理。

- **组合**。 将上述两种风格结合起来。例如，您可以使用线程池接收请求和发送响应，并使用按需线程获取大型页面文件。

可以自由选择以上三种中的任何一种，或者使用您认为很酷的其他多线程架构。

### 3.4 支持HTTP管道化

在基本版本中，**在同一时间内每个TCP连接仅有一个请求**。客户端等待响应，当收到响应时，可能重用TCP连接进行新请求（或使用新的TCP连接）。这也是正常的HTTP服务器支持的方式。

在高级版本中，**可以在同一TCP连接上同时发出多个HTTP请求**。这也称为HTTP管道化，许多真实的浏览器和服务器（如Chrome）都支持。请注意，来自同一TCP连接的HTTP请求应以相同的顺序进行响应。因此，在使用复杂的多线程样式时，请注意顺序问题。

### 3.5 指定参数

您的程序应启用长选项以接受参数，并在启动期间指定这些参数。

它们是：

| 参数 | 描述 |
| --- | --- |
| -i, --ip \<IP\> | 指定服务器IP地址。 |
| -p, --port \<PORT\> | 选择HTTP服务器监听传入连接的端口。 |
| --proxy \<PROXY\> | 选择要代理的“上游”HTTP服务器。 |
| -t, --threads \<

THREADS\> | 如果使用多线程，请指定线程数。 |

`--proxy`可以在`://`之前有模式，并且在冒号后有端口号（例如`http://www.example.com:80`）。如果未指定端口号，则80端口是HTTP的默认端口。

如果您不了解*长选项*，可以阅读[这里](https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html#Argument-Syntax)。您可能需要使用一些函数，如`getopt_long()`，`getopt_long_only()`，`getopt()`等。使用`man`命令检查这些函数的用法。

### 3.6 运行您的HTTP服务器

**对于高级版本**：

您的程序应能够在终端上启动。如果您的程序称为*http-server*，只需键入：

在非代理模式下：

`./http-server --ip 127.0.0.1 --port 8888 --threads 8` 

这意味着您的HTTP服务器的IP地址是127.0.0.1，服务端口是8888。 --number-thread表示线程池中有8个线程，用于处理多个客户端请求。

在代理模式下：

`./http-server --ip 127.0.0.1 --port 8888 --threads 8 --proxy http://www.example.com:80`

这意味着这是一个HTTP代理服务器。此代理的IP地址为127.0.0.1，服务端口为8888。 --proxy表示“上游”HTTP服务器是`http://www.example.com:80`。因此，如果向此代理发送请求消息（即`127.0.0.1:8888`），它将将此请求消息转发到“上游”HTTP服务器（即`http://www.example.com:80`）并将响应消息转发到客户端。

> 如果要从其他主机访问此服务器，可能需要打开与端口对应的防火墙并将IP绑定到0.0.0.0。

当您运行上述命令时，您的HTTP服务器应正常运行。

### 3.7 访问您的HTTP服务器

我们假设服务器绑定的IP是`127.0.0.1`，端口是`8080`。如果使用了代理功能，则远程服务器是`www.example.com`。

为了更有效地测试和评分，我们需要您实现以下接口的一些部分，并确保响应的内容与我们的期望一致。

**这就是为什么我们提供一些标准静态文件的原因。您可以在`./static/`和`./data/`中检查这些文件。**

请确保您的服务器支持访问它们，并**不要修改它们的内容或相对路径**。

#### 3.7.1 使用GET方法

**1) 访问静态文件**

对于**基本**版本：

| 路径 | 本地文件系统中的文件 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /, /index.html | /`{static dir}`/index.html | 200 | text/html |
| /404.html | /`{static dir}`/404.html | 404 | text/html |
| /501.html | /`{static dir}`/501.html | 501 | text/html |
| 任何其他错误路径 | /`{static dir}`/404.html | 404 |text/html |

例如：

```shell
user@linux:~/http-server$ curl -i -X GET http://localhost:8080/index.html
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 210

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <link rel="stylesheet" href="/styles.css"/>
    <title>Http Server</title>
</head>
<body>
    <h1>index.html</h1>
</body>
</html> 
```

对于**高级**版本：

| 路径 | 本地文件系统中的文件 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /, /index.html | /`{static dir}`/index.html | 200 | text/html |
| /404.html | /`{static dir}`/404.html | 404 | text/html |
| /501.html | /`{static dir}`/501.html | 501 | text/html |
| [/\*]/\*.html | /`{static dir}`[/\*]/\*.html | 200 | text/html |
| [/\*]/\*.js | /`{static dir}`[/\*]/\*.js | 200 | text/javascript |
| [/\*]/\*.css | /`{static dir}`[/\*]/\*.css | 200 | text/css |
| [/\*]/\*.json | /`{static dir}`[/\*]/\*.json | 200 | application/json |
| 任何其他错误路径 | /`{static dir}`/404.html | 404 | text/html |

```
user@linux:~/http-server$ curl -i -X GET http://localhost:8080/data.json
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 93

[{"id":1,"name":"Foo"},{"id":2,"name":"Bar"},{"id":3,"name":"Foo Bar"},{"id":4,"name":"Foo"}]
```

> 为了进行标准化测试，在不需要的空格的情况下将JSON数据输出为一行。

**2) 访问用于获取数据的Web服务**

对于**基本**版本：

| 路径 | 获取数据 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /api/check | `data.txt`中的字符串 | 200 | text/plain |
| 任何其他错误路径 | /{static files}/404.html | 404 | text/html |

例如：

```shell
user@linux:~/http-server$ curl -i -X GET http://localhost:8080/api/check
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 13

id=1&name=Foo
```

对于**高级**版本：

| 路径 | 获取数据 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /api/list | `data.json`中的所有对象 | 200 | application/json
| /api/check?id=1&name=Foo | `data.json`中的对象（id=1，name=Foo） | 200 | application/json |
| /api/check?id=5&name=NotExist | `data.json`中的对象（id=5，name=NotExist） | 404 | application/json |
| 任何其他错误路径 | /{static files}/404.html | 404 | text/html |

```shell
user@linux:~/http-server$ curl -i -X GET http://localhost:8080/api/list
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 129

[{"id":1,"name":"Foo"},{"id":2,"name":"Bar"},{"id":3,"name":"Foo Bar"},{"id":4,"name":"Foo"}]
```

#### 3.7.2 使用POST方法

对于**基本**版本：

| 路径 | 上传数据 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /api/upload | `id=1&name=Foo` | 200 | text/plain |
| /api/upload?id=1&name=Foo | `id=1&name=Foo` | 200 | text/plain |
| /api/upload?id=1&name=Foo&extra=bar | `id=1&name=Foo&extra=bar` | 200 | text/plain |
| /api/upload?id=1&name=NotExist | `id=1&name=NotExist` | 404 | text/plain |
| 任何其他错误路径 | /{static files}/404.html | 404 | text/html |

例如：

```shell
user@linux:~/http-server$ curl -i -X POST http://localhost:8080/api/upload -d "id=1&name=Foo"
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 13

id=1&name=Foo
```

对于**高级**版本：

| 路径 | 上传数据 | 状态码 | 内容类型 |
| --- | --- | --- | --- |
| /api/upload | `id=1&name=Foo` | 200 | application/json |
| /api/upload?id=1&name=Foo | `id=1&name=Foo` | 200 | application/json |
| /api/upload?id=1&name=Foo&extra=bar | `id=1&name=Foo&extra=bar` | 200 | application/json |
| /api/upload?id=1&name=NotExist | `id=1&name=NotExist` | 404 | application/json |
| 任何其他错误路径 | /{static files}/404.html | 404 | text/html |

```shell
user@linux:~/http-server$ curl -i -X POST http://localhost:8080/api/upload -d "id=1&name=Foo"
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 26

{"id":1,"name":"Foo","success":true}
```

### 3.8 附加说明

1. 请注意，对于GET请求，您的服务器应正确解析查询字符串，并从中提取参数。参数应作为键值对映射传递给您的处理程序。

2. 请注意，为了进行标准化测试，我们可能会使用简单的静态文件，而不是大型静态文件或特殊的Web服务。

3. 对于高级版本，考虑到要从其他服务器代理请求，您的服务器应具有适当的错误处理机制，以便它可以处理服务器不可用或未响应的情况。这包括但不限于代理服务器的连接错误，超时等。

4. 请注意，如果您的服务器以代理模式运行，并且上游HTTP服务器不可用或不响应，您的服务器应返回`502 Bad Gateway`状态代码和适当的错误消息。

5. 请注意，您可能需要处理HTTP管道化，因为客户端可以在同一TCP连接上同时发送多个HTTP请求。处理HTTP管道化的一种方式是，服务器可能在每个TCP连接上启动一个新的线程，以处理来自客户端的新HTTP请求。因此，请确保在每个TCP连接上都可以处理HTTP管道化请求，而不是在整个服务器中。

6. 考虑对您的服务器进行一些基本的压力测试。有关此类测试的提示，请参见[这里](http://www.cs.unc.edu/~jeffay/dirt/FAQ/Performance_testing_faq.html)。

7. 请确保您的服务器支持并充分利用HTTP/1.1 Keep-Alive功能，以避免在每个请求/响应周期中重新建立连接。

8. 当您的服务器成功提供请求时，响应的状态行应为`HTTP/1.1 200 OK`。当出现错误时，响应的状态行应包含适当的错误代码，例如`HTTP/1.1 404 Not Found`或`HTTP/1.1 501 Not Implemented`。确保在响应中包括正确的HTTP标头，例如`Content-Type`。

9. 您可以假设请求是有效的HTTP/1.1请求，并且不会有太多的垃圾数据。如果有很多无效的HTTP请求（例如，非法字符，非法方法等），您可以选择是否忽略它们或返回适当的错误代码。

10. 请注意，您不需要实现以下功能：
   - Cookie处理
   - HTTP缓存
   - 持久性连接（Keep-Alive）的超时
   - Chunked编码
   - gzip或其他HTTP消息编码

11. 请确保在HTTP响应中正确处理Content-Length标头，以确保客户端能够正确读取响应的所有数据。

12. 您可能需要实现HTTP/1.1的一部分，以便在适当的时候支持`Connection: close`头部。

13. 考虑使用简单但功能齐全的`curl`工具进行测试。可以使用以下命令从终端发出GET和POST请求：

   ```bash
   # 发送GET请求
   curl -i -X GET http://localhost:8080

   # 发送POST请求
   curl -i -X POST http://localhost:8080/api/upload -d "id=1&name=Foo"
   ```

   如果您在本地测试，

将`localhost`和`8080`替换为您的服务器的IP地址和端口。

14. 在实现代理服务器功能时，考虑到一些安全问题，例如防止您的服务器被滥用成为中间人攻击。这可能需要一些额外的安全措施。

15. 请确保您的服务器在适当的情况下处理各种HTTP请求方法，如GET和POST。对于不支持的HTTP方法，请返回`501 Not Implemented`状态。

16. 对于POST请求，服务器应正确解析请求主体，并从中提取数据。

17. 考虑并发性和性能。您的服务器应能够同时处理多个并发客户端请求，并在高负载情况下保持良好的性能。

18. 您的服务器应该能够在指定的IP地址和端口上侦听传入连接。如果没有指定IP地址，您的服务器应该侦听所有可用的网络接口。

19. 在高级版本中，确保代理服务器能够正确处理不同类型的请求，包括GET和POST。代理服务器应该能够将请求转发到上游HTTP服务器，并将响应返回给客户端。

20. 请确保您的服务器在收到请求时能够正确解析HTTP请求行和标头，并能够根据请求的内容生成适当的响应。

21. 考虑错误处理和日志记录。如果发生错误，服务器应该返回适当的错误代码，并记录错误消息以便进行故障排除。

22. 请注意，您的服务器不需要支持HTTPS，只需要处理普通的HTTP请求。

23. 在处理HTTP请求时，请考虑安全性和防范常见的攻击，如跨站脚本（XSS）和跨站请求伪造（CSRF）。您不需要实现复杂的安全功能，但确保您的服务器不容易受到常见攻击是一个好主意。

24. 最后但同样重要的是，请确保您的服务器的代码易于理解和维护。良好的文档、适当的注释和模块化的设计都是写出可维护代码的重要因素。

## 第4部分：评分和提交

### 4.1 评分

您的项目将根据以下几个方面进行评分：

1. **基本功能完成度（40分）**：
   - 实现基本版本中的所有要求，包括正确处理GET和POST请求，支持静态文件访问，实现基本的Web服务，正确处理路径和错误情况，以及适当的HTTP响应。

2. **高级功能完成度（40分）**：
   - 如果您选择实现高级版本，请确保您的服务器成功实现了所有高级要求，包括代理服务器功能，多线程支持，HTTP管道化和指定参数的功能。

3. **代码质量和可读性（10分）**：
   - 您的代码应该是清晰、有组织的，包括适当的注释。遵循良好的编码实践，确保代码易于理解和维护。

4. **错误处理和鲁棒性（10分）**：
   - 您的服务器应该能够正确处理各种错误情况，并在发生错误时返回适当的HTTP状态代码和错误消息。服务器应该具有一定的鲁棒性，能够在各种情况下正常运行。

### 4.2 提交指南

完成项目后，请将以下内容打包成一个压缩文件进行提交：

1. 项目的完整源代码，包括所有必要的文件和目录结构。

2. 项目的文档，包括说明如何构建、运行和测试您的服务器的说明。

3. 项目的README文件，其中包含任何额外的说明、注意事项或特殊说明。

4. 任何其他必要的文件，如测试脚本、配置文件等。

请确保您的提交文件易于阅读和理解，并按照上述要求组织。完成后，您可以将项目提交到适当的提交渠道，例如通过在线平台或通过电子邮件。

### 4.3 项目截止日期

请在规定的截止日期前提交您的项目。如果您在截止日期之前完成了项目，请提前提交。

### 4.4 项目评审

完成项目的评审过程可能需要一些时间。请耐心等待评审结果，并确保您能够接收到相关的反馈和评分。如果有需要，我们可能会与您联系以进行更多的讨论或澄清。

祝您好运，希望您能够成功完成这个项目！
