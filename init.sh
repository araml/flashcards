#!/bin/bash
rm -rf sqlite
mkdir sqlite -p
cd sqlite
curl https://www.sqlite.org/2020/sqlite-amalgamation-3320300.zip -o sql.zip
unzip sql.zip
cd */
mv * ..
cd ..
gcc sqlite3.c -c -o sqlite.o
ar rcs sqlite.a sqlite.o
rm sqlite.o

