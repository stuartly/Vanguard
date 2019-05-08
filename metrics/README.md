# README #

### How do I get set up? ###
* 安装Joern
基本可以参照joern的官方文档，链接见下：
http://joern.readthedocs.io/en/latest/installation.html
需要注意的几点：

1. 选择Java 1.8版本

2. Neo4J Server请选用官方建议的Neo4J 2.1.5 Community Edition
Neo4J的官网好像无法直接找到2.X版本的下载，可以用以下命令得到安装包
> wget http://dist.neo4j.org/neo4j-community-2.1.5-unix.tar.gz

3. 安装python-joern时，请选择py2neo 2.0版本

* 如何跑程序

1. 首先要把被测程序导入数据库(可以参照以下链接)
http://joern.readthedocs.io/en/latest/tutorials/unixStyleCodeAnalysis.html#importing-the-code

cd $JOERN
rm -rf .joernIndex
./joern tutorial/vlc-2.1.4/ #source code of project


2. 开启Neo4J数据库
cd $NEO4J
$NEO4J/bin/neo4j stop
> $NEO4J/bin/neo4j start-no-wait

3. 执行functionMetrics.py
