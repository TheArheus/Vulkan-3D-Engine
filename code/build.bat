rem @echo off

echo "Builling everything"

pushd engine
call build.bat
popd
REM if %ERRORLEVEL% neq 0 (echo Error:%ERRORLEVEL% && exit)

pushd testbet
call build.bat
popd
REM if %ERRORLEVEL% neq 0 (echo Error:%ERRORLEVEL% && exit)

echo "All assemblies build successfully."
