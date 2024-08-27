if [[ ! $(which wmake) ]]; then
  echo "wmake not found, is it installed and did you source setenvwc.sh?"
  exit 1
fi

mkdir WC16
wmake -f makefile.w16 WC16/DOOMTDWC.EXE
rm *.err
