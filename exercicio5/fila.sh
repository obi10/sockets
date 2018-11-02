pids=""
for i in {1..4}
do
    ./cliente 127.0.0.1 8000 &
    pids="$pids $!"
done

for pid in $pids; do
    wait $pid
done
