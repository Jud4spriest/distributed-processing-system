echo "Calling workers.."

./worker 127.0.0.1 &
./worker 127.0.0.1 &
./worker 127.0.0.1 &
./worker 127.0.0.1 &
./worker 127.0.0.1 &
./worker 127.0.0.1 &
./worker 127.0.0.1 &

sleep 10

echo "Calling client.."
./client 127.0.0.1 add 1 8 &
./client 127.0.0.1 multiply 2 6 & 
./client 127.0.0.1 add 3 7 & 
./client 127.0.0.1 multiply 4 7 &
./client 127.0.0.1 add 5 5 &
./client 127.0.0.1 add 6 3 &
./client 127.0.0.1 add 7 23 &
./client 127.0.0.1 multiply 8 7 &
./client 127.0.0.1 add 1 8 &
./client 127.0.0.1 add 2 6 & 
./client 127.0.0.1 subtract 13 7 & 
./client 127.0.0.1 add 4 7 &
./client 127.0.0.1 multiply 5 5 &
./client 127.0.0.1 add 6 3 &
./client 127.0.0.1 add 7 23 &
./client 127.0.0.1 divide 8 7