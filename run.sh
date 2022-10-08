g++ -o t tomasolo.cpp -std=c++11
g++ -o asm asm.cpp -std=c++11
./asm ./sample/input1.asm ./sample/output1.txt
./asm ./sample/input2.asm ./sample/output2.txt
./asm ./sample/input3.asm ./sample/output3.txt
./t ./sample/output1.txt > 1.txt
./t ./sample/output2.txt > 2.txt
./t ./sample/output3.txt > 3.txt