echo "Calling workers.."

./worker 127.0.0.1 &
# ./worker 127.0.0.1 &
# ./worker 127.0.0.1 &
# ./worker 127.0.0.1 &
# ./worker 127.0.0.1 &
# ./worker 127.0.0.1 &
# ./worker 127.0.0.1 &

sleep 2

IP="127.0.0.1"
MIN=1
MAX=100
FREQ=$1

T=$(echo "scale=5; 1 / $FREQ" | bc)

while true; do
  NUM1=$(shuf -i $MIN-$MAX -n 1)
  NUM2=$(shuf -i $MIN-$MAX -n 1)

  ./client $IP add $NUM1 $NUM2
  ./client $IP subtract $NUM1 $NUM2
  ./client $IP multiply $NUM1 $NUM2
  ./client $IP divide $NUM1 $NUM2
  
  sleep $T
done