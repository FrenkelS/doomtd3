if "%WATCOM%" == "" goto error

mkdir WC16
wmake -f makefile.w16 WC16\DOOMTDWC.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end
