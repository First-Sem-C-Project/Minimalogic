@echo off
pushd src
lualatex report.tex 
xcopy /Y report.pdf ..\
popd

