@echo off
set curdir=%cd%

set var=login_server.exe gateway_server.exe scene_server.exe db_proxy_server.exe service_registry.exe 
for %%a in (%var%) do (
	tasklist /nh | findstr /i %%a > NUL  
	if ErrorLevel 0 (
	  taskkill /f /t /im "%%a"
	) else (
	  echo not exist %%a
	)
)

pause
