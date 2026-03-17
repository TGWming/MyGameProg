@echo off

rem E:\S3\S3_all\voice\cn
set dir=cn
echo d | xcopy "../voice/%dir%/*" "../pro/Assets" /e /y /f

pause