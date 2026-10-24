#!/system/bin/sh

# Переходим в папку со скриптом
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

echo "======================================"
echo "   ESP LAUNCHER (MT Manager)  "
echo "======================================"

# Убиваем старые процессы, если они висят
killall pubg_cheat 2>/dev/null
killall com.cheat.overlay 2>/dev/null
sleep 1

# Запускаем приложение-оверлей
echo "[*] Запускаем оверлей..."
am start -n com.cheat.overlay/.MainActivity

# Ищем игру
PID=$(pidof com.tencent.ig)
if [ -z "$PID" ]; then
    echo "[-] ОШИБКА: PUBG Mobile не запущен!"
    sleep 5
    exit 1
fi
echo "[+] Найден PID игры: $PID"

# Отключаем SELinux
echo "[*] Отключаем SELinux..."
setenforce 0

# Даем права и запускаем в фоне
chmod 755 ./pubg_cheat
echo "[*] Запускаем чтение памяти..."

# Пишем логи в файл log.txt
nohup ./pubg_cheat --pid $PID --offsets ./offsets.json > log.txt 2>&1 &

echo "[+] УСПЕХ! Чит работает в фоне."
echo "[*] Логи пишутся в log.txt"
sleep 3
