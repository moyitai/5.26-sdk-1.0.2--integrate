@echo off
if not exist project\preview md project\preview
cd project\preview
if not exist ResBuilder.xml copy ..\..\..\..\UITools\ResBuilder.xml
start "" ..\..\..\..\UITools\Ԥ��ͼ���ɹ���.exe