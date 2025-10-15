
PORT="$1"
LOG="$2"
PIDSTAT_OPTS="$3"

# Start the server
./servidor "$PORT" &
SERVER_PID=$!

# Start pidstat
pidstat $PIDSTAT_OPTS -p $SERVER_PID 1 > "$LOG" &
PIDSTAT_PID=$!

# On interrupt (e.g. Ctrl+C), kill both processes
trap "echo 'Interrupted. Killing server and pidstat...'; kill $SERVER_PID $PIDSTAT_PID 2>/dev/null; exit 0" INT TERM

# Wait for pidstat to exit (e.g., if you stop it manually)
wait $PIDSTAT_PID

# If pidstat exits, kill the server too
kill $SERVER_PID 2>/dev/null || true