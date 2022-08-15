@echo on


set curdir=%cd%
%curdir%/MergeFiles.exe SyncSql.yaml

echo Exit Code is %errorlevel%

pause


