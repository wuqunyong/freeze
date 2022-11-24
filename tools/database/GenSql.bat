@echo on


set curdir=%cd%
python %curdir%/GenSql.py %curdir%/GenSql.yaml

clang-format -i %curdir%/dao/apie/*.h
clang-format -i %curdir%/dao/apie_account/*.h

echo Exit Code is %errorlevel%

pause
