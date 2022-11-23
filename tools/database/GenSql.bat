@echo on


set curdir=%cd%
python %curdir%/GenoratorSql.py %curdir%/GenSql.yaml

clang-format -i %curdir%/*.h

echo Exit Code is %errorlevel%

pause
