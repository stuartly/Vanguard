# README #

### How do I get set up? ###
* Install 

http://joern.readthedocs.io/en/latest/installation.html

Note:
1. Java: choose Java 1.8 version

2. Neo4J Server: choose Neo4J 2.1.5 Community Edition
> wget http://dist.neo4j.org/neo4j-community-2.1.5-unix.tar.gz

3. python-joern: choose py2neo 2.0 version

* Usage

1. http://joern.readthedocs.io/en/latest/tutorials/unixStyleCodeAnalysis.html#importing-the-code

cd $JOERN
rm -rf .joernIndex
./joern tutorial/vlc-2.1.4/ #source code of project


2. start Neo4J
cd $NEO4J
$NEO4J/bin/neo4j stop
> $NEO4J/bin/neo4j start-no-wait

3. python functionMetrics.py
