@echo off
del /q "../project/*"
rmdir "../project/.vs" /s /q
rmdir "obj" /s /q
@echo on
premake5.exe vs2017