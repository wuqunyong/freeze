@echo on


set curdir=%cd%
%curdir%/SyncSql.exe %curdir%/SyncSql.yaml

echo Exit Code is %errorlevel%

pause
