To start the server:
  g++ Server1.cpp -o Server1 -lws2_32
  ./Server1
After start the clients:
  g++ Client1.cpp -o Client1 -lws2_32 "-Wl,-subsystem,console"
  ./Client1
If you run from 1 pc, then you need to run it in different windows:
  g++ Client1.cpp -o Client2 -lws2_32 "-Wl,-subsystem,console"
  ./Client2
Next:
  g++ Client1.cpp -o Client3 -lws2_32 "-Wl,-subsystem,console"
  ./Client3
