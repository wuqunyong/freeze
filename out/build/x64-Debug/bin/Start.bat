call :startfunc "service_registry" service_registry.exe ../../../../conf/service_registry.yaml
call :startfunc "db_account_proxy" db_proxy_server.exe ../../../../conf/db_account_proxy.yaml
call :startfunc "db_role_proxy" db_proxy_server.exe ../../../../conf/db_role_proxy.yaml
call :startfunc "login_server" login_server.exe ../../../../conf/login_server.yaml
call :startfunc "gateway_server" gateway_server.exe ../../../../conf/gateway_server.yaml
call :startfunc "scene_server" scene_server.exe ../../../../conf/scene_server.yaml

goto end

:startfunc
	tasklist /nh | findstr /i %1 > NUL  
	if ErrorLevel 1 (
	  start %1 %2 %3
	) else (
	  echo exist %1 %2 %3
	)
	rem ping 127.0.0.1 -n 6 > nul
	
goto :eof

:end