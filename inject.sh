#!/system/bin/sh
PKG="com.tencent.ig"
ACT="com.epicsgames.ue4.GameActivity"   # проверь, у тебя выше было com.epicgames...
TARGET_DIR="/data/data/$PKG"
INJECT_BIN="inject"
SO_FILE="libpubgcheat.so"

RAND=$(mktemp -u XXXXXXXX)
NEW_SO="libpubgcheat_${RAND}.so"
NEW_INJ="inject_${RAND}"

cp "$INJECT_BIN" "${TARGET_DIR}/${NEW_INJ}"
cp "$SO_FILE"   "${TARGET_DIR}/${NEW_SO}"
chmod 777 "${TARGET_DIR}/${NEW_INJ}" "${TARGET_DIR}/${NEW_SO}"

am start -n "$PKG/$ACT"
sleep 2.5

"${TARGET_DIR}/${NEW_INJ}" -n "$PKG" -so "${TARGET_DIR}/${NEW_SO}"

rm -f "${TARGET_DIR}/${NEW_INJ}" "${TARGET_DIR}/${NEW_SO}"
