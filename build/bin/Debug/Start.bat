call :startfunc "ServiceRegistry" service_registry.exe ../../../conf/service_registry.yaml

timeout /nobreak /t 3

call :startfunc "AccountDBProxy" db_proxy_server.exe ../../../conf/db_account_proxy.yaml
call :startfunc "RoleDBProxy" db_proxy_server.exe ../../../conf/db_role_proxy.yaml
call :startfunc "LoginServer" login_server.exe ../../../conf/login_server.yaml
call :startfunc "GatewayServer" gateway_server.exe ../../../conf/gateway_server.yaml
rem call :startfunc "RouteProxy" RouteProxy.exe ../../conf/route_proxy.yaml
call :startfunc "SceneServer" scene_server.exe ../../../conf/scene_server.yaml

goto end

:startfunc
	tasklist /nh | findstr /i %1 > NUL  
	if ErrorLevel 1 (
	  start %1 %2 %3
	) else (
	  echo exist %1 %2 %3
	)
	
goto :eof

:end